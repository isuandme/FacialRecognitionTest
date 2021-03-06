#include "opencv2/core.hpp"
#include "opencv2/face.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgcodecs.hpp"

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <dirent.h>
#include <stdexcept>
#include <mach-o/dyld.h>
#include <ctype.h>

using namespace std;
using namespace cv;
using namespace cv::face;

static void read_images(const string& directory, vector<Mat>& images, vector<int>& labels){
    
    DIR* dir;
    dir = opendir(directory.c_str());
    
    if(dir == NULL){
        cout << "Directory didn't not have anything in it ... please check." << endl;
        return;
    } else {
        cout << "Directory  " << directory << "  is opened." << endl;
    }
    
    dirent* di;
    int label_count = 0;
    
    while((di = readdir(dir)) != NULL){
        if(di->d_name[0] == 's'){
            cout << "Directory:  " << di->d_name << endl;
            DIR* cur_dir;
            const string& cur_dir_name = directory + di->d_name + "/";
            cout << "   cur_dir_name  -->  " << cur_dir_name << endl;
            cur_dir = opendir(cur_dir_name.c_str());
            if(cur_dir == NULL){
                cout << " ------- Sub directory " << cur_dir_name << " failed to initialize in att_face folder ------- " << endl;
                return;
            }
            
            dirent* dj;
            while((dj = readdir(cur_dir)) != NULL){
                // __uint16_t d_namlen;     /* length of string in d_name */
                if(dj->d_namlen > 3){
                    cout << "       File Name:  " << dj->d_name << endl;
                    const string& cur_file_path = cur_dir_name + dj->d_name;
                    cout << "current working directory : " << cur_file_path << endl;
                    Mat img = imread(cur_file_path, IMREAD_GRAYSCALE);
                    cout << "Label Count: " << label_count << endl;
                    if(!img.data){
                        cout << "Image at ----    " << cur_file_path << "   ---- is empty. Please check file" << endl;
                        exit(1);
                    }
                    images.push_back(img);
                    labels.push_back(label_count);
                }
            }
            label_count++;
        }
    }
    return;
}

int main(int argc, char * argv[]){

    string output_folder = ".";
    string sample_folder = ".";
    vector<Mat> images;
    vector<int> labels;
    
    if(argc < 2){
        cout << " <input_folder> <output_folder> " << endl;
        exit(1);
    }
    
    if(argc > 3 ){
        cout << " <csv.ext> <output_folder> ... DUDE figure it out you drunk " << endl;
        exit(1);
    }
    
    string csv_file = string(argv[1]);
    
    if(argc == 3){
        sample_folder = string(argv[1]);
        output_folder = string(argv[2]);
    }
    
    read_images(sample_folder, images, labels);
    
    //int height = images[0].rows;
    
    if(images.size() <= 1) {
        string error_message = "Image set is not large enough to proceed please inrease dtat size.";
        CV_Error(CV_StsError, error_message);
    }
    
    int im_width = images[0].cols;
    int im_height = images[0].rows;
    
    //Ptr< FisherFaceRecognizer >     create (int num_components=0, double threshold=DBL_MAX)
    Ptr<FaceRecognizer> model =  FisherFaceRecognizer::create();
    model->train(images, labels);
    
    CascadeClassifier haar_cascade;
    haar_cascade.load("/Users/jacobpratt/Programming/OpenCV/opencv-3.3.1/data/haarcascades/haarcascade_frontalface_alt.xml");
    
    // Get a handle to the Video device:
    VideoCapture cap(0);
    if(!cap.isOpened()) {
        cerr << "Capture Device ID cannot be opened." << endl;
        return -1;
    }
    
    Mat frame;
    
    for(;;){
        cap>>frame;
        Mat original = frame.clone();
        Mat gray;
        cvtColor(original, gray, CV_BGR2GRAY);
        vector< Rect_<int> > faces;
        haar_cascade.detectMultiScale(gray, faces);
        for(int i = 0; i < faces.size(); i++) {
            
            Rect face_i = faces[i];
            
            Mat face = gray(face_i);
            
            Mat face_resized;
            cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);
            
            int prediction = model->predict(face_resized);
            
            rectangle(original, face_i, CV_RGB(0, 255,0), 1);
            
            string box_text = format("Prediction = %d", prediction);
            
            int pos_x = std::max(face_i.tl().x - 10, 0);
            int pos_y = std::max(face_i.tl().y - 10, 0);
            
            putText(original, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
        }
        
        imshow("face_recognizer", original);
        
        char key = (char) waitKey(20);
        
        if(key == 27)
            break;
    }
    
        
    return 0;
}
