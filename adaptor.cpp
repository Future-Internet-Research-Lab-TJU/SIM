#include "adaptor.hpp"
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

//"adaptor" is used to process data set files and obtain information about each edge in the dataset.
//The incoming "buffersize" indicates the size of the buffer. The "buffersize" holds the data for each line in the data set file.
Adaptor::
Adaptor(std::string filename, uint64_t buffersize) {
    data = (adaptor_t*)calloc(1, sizeof(adaptor_t)); //The "calloc" function is used to open up space.
    data->databuffer = (unsigned char*)calloc(buffersize, sizeof(unsigned char));
    data->ptr = data->databuffer;  //A pointer to the data currently being read from the databuffer.
    data->cnt = 0;  //Look at how many edges are in the dataset, that is, how many lines are in the file.
    data->cur = 0;  //Look at what line is being read.

    //Read data set file
    std::ifstream infile;
    infile.open(filename);
    if(!infile.is_open())
    {
        cout << " dataset open false" <<endl;
    }
    std::string srcdst;
    unsigned char* p = data->databuffer;  //Stores the data of an edge, including source and destination IP and size。
    int frequency = 1;
    //Gets the contents of the data set
    while (getline(infile, srcdst))
    {
        int pos1 = srcdst.find(" ");     //Locate the positions of space
        int pos2 = srcdst.find("\r\n");  //Locate the positions of carriage returns and newlines
        std::string ip1 = srcdst.substr(0,pos1); //Get the source IP
        std::string ip2 = srcdst.substr(pos1+1,pos2-pos1);//Get the destination IP 
        int srcip = atoi(ip1.c_str());   //The "atoi()" function converts strings to integers
        int dstip = atoi(ip2.c_str());
        memcpy(p, &srcip, sizeof(uint32_t));
        memcpy(p+sizeof(uint32_t), &dstip, sizeof(uint32_t));
        memcpy(p+sizeof(uint32_t)*2, &frequency, sizeof(uint16_t));//Save the data set with frequency 1
        p += sizeof(uint32_t)*2+sizeof(uint16_t);
        data->cnt++;  //Look at how many edges there are in the dataset, that is, how many rows there are.
    }
    infile.close();
}

Adaptor::~Adaptor() {
    free(data->databuffer);
    free(data);
}

//Input a struct pointer to record the edge, store the current edge source and destination IP and edge weight into the struct
//And store the edge information in the data set file into the variable "t"
int Adaptor::GetNext(tuple_t* t) {
    if (data->cur > data->cnt) {
        return -1;
    }
    t->key.src_ip = *((uint32_t*)(data->ptr));
    t->key.dst_ip = *((uint32_t*)(data->ptr+4));
    t->size = *((uint16_t*)(data->ptr+8));
    data->cur++;
    data->ptr += 10;
    return 1;
}

void Adaptor::Reset() {
    data->cur = 0;
    data->ptr = data->databuffer;
}

uint64_t Adaptor::GetDataSize() {
    return data->cnt;
}

