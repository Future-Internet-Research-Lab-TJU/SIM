//
// Created on 2022/9/1.
//


#ifndef _SIM_LIGHT_H_
#define _SIM_LIGHT_H_

#include<cmath>
#include<string>
#include<iostream>
#include<memory.h>
#include<queue>
#include <map>
#include "datatypes.hpp"
using namespace std;

extern "C"
{
#include "hash.h"
}

typedef uint8_t l_count;


class SIM_light
{
public:
    //Parameter Definitions
    int length;
    int width;
    l_count *arr_light;  //Defines a pointer to the light counter
    l_count l_weight;
    bool light_full;
	uint32_t light_full_num;  //Determine overflow times
    
    SIM_light() 
    {
        ;
    }

    void Reset()
    {
        //Initialize the data structure to 0.
        memset(arr_light, 0, sizeof(l_count) * length * width); 
    }

    void Build_light(int l, int w);

    ~SIM_light()
    {
        delete[] arr_light;
    }

    void insert(unsigned int index_l, val_tp fint);

    uint32_t L_EdgeQuery(unsigned int index_l);

    uint32_t L_NodeQuery(unsigned int index_l);

};

void SIM_light::Build_light(int l, int w)
{
    length = l;
    width = w;
    arr_light = new l_count[length * width];  //Build the physical structure of sim_light.

    light_full_num = 0;

    //Initialize the data structure to 0.
    memset(arr_light, 0, sizeof(l_count) * l * w);  

}

//Data structure insertion function, "s1"/"s2" are the source/destination nodes,  "fint" is the edge weight size.
void SIM_light::insert(unsigned int index_l, val_tp fint)
{ 
 
    light_full = false;
   
	if(arr_light[index_l] + fint > 255)
    {
		light_full_num++;
        light_full = true;
    }
    else
        arr_light[index_l] += fint;
    
    l_weight = arr_light[index_l];

}

uint32_t SIM_light::L_EdgeQuery(unsigned int index_l)
{
    return arr_light[index_l];
}

uint32_t SIM_light::L_NodeQuery(unsigned int index_l)
{
    uint32_t L_result = 0;

    for(int i = 0; i < length; i++)
    {               
        L_result += arr_light[index_l+i];
    }

    return L_result;    
}


#endif //_SIM_H_
