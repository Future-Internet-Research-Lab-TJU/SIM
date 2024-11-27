//
// Created on 2022/9/1.
//


#ifndef _SIM_HEAVY_H_
#define _SIM_HEAVY_H_

#include<cmath>
#include<string>
#include<iostream>
#include<memory.h>
#include<queue>
#include <map>
#include "datatypes.hpp"

using namespace std;

#define num_16 29  //In the heavy part, the number of 16bits cells in a bucket.
#define num_32 7  //In the heavy part, the number of 32bits cells in a bucket.
//#define cell_n num_16+num_32  //In the heavy part, the total number of cells in a bucket.

struct Cell_16
{
    uint32_t key[2];
    uint16_t count;
};

struct Cell_32
{
    uint32_t key[2];
    uint32_t count;
};

struct Bucket
{
    Cell_16 cell_16[num_16];
    Cell_32 cell_32[num_32];
};

class SIM_heavy
{
   
public:

    //Parameter Definitions
    int length;   //Define the length of the heavy part
    int width;    //Define the width of the heavy section
    Bucket *arr_heavy;  //Define a bucket pointer to index the location of the heavy part bucket
    uint32_t hc_min;   //Define a minimum weight counter to record the minimum value of a cell in a bucket
    uint8_t min_num;   //Records the position of the minimum weight cell
    bool flog_false;  //The flag indicates that the insertion has failed in heavy part
    uint32_t full_32;

    //int cell_n = num_16+num_32;


    SIM_heavy() 
    {
        ;
    }

    void Build_heavy(int l, int w);

    ~SIM_heavy()
    {
        delete[] arr_heavy;
    }


    //Data structure insertion function, "s1"/"s2" are the source/destination nodes,  "fint" is the edge weight size.
    bool insert(unsigned int index_h, uint32_t * key, val_tp fint);

    uint32_t H_EdgeQuery(unsigned int index_h, uint32_t * key);

    uint32_t H_NodeQuery(unsigned int index_h, uint32_t * key);

    void H_HH_Edgequery(val_tp thresh, myvector& HH_Edge);

    void H_HH_Nodequery(myset& candidate_node);

    void H_HH_Nodequery(val_tp thresh, myset& candidate_node);
    
    void Reset();

    void heavay_print(myvector& HH_Edge_16, myvector& HH_Edge_32);

};

void SIM_heavy::Build_heavy(int l, int w)
{
    full_32 = 0;
    length = l;
    width = w;
    arr_heavy = new Bucket[length * width];  //Build the physical structure of a layer of two-dimensional matrix, that is, the physical structure of sim_heavy.

    //Initialize the SIM data structure to 0.
    memset(arr_heavy, 0, sizeof(Bucket) * length * width);  
}

//Data structure insertion function, "s1"/"s2" are the source/destination nodes,  "fint" is the edge weight size.
bool SIM_heavy::insert(unsigned int index_h, uint32_t * key, val_tp fint)
{
    //Variable predefinition
    uint8_t min_num16 = 0, min_num32 = 0;
    flog_false = false;
    uint32_t key_temp[2];
    uint32_t weight_temp;

    min_num = 0;  //Records the minimum weight cell in each bucket.

    for (int i = 0; i < num_32; ++i)
    {
        //key[0] is the source ID，key[1] is the destination ID, the "stoi()" function converts a string to an int
        if ((arr_heavy[index_h].cell_32[i].key[0] == *key && (arr_heavy[index_h].cell_32[i].key[1] == *(key + 1))))  
        {
            arr_heavy[index_h].cell_32[i].count += fint;
            return flog_false;
        } 
        else 
            if ((arr_heavy[index_h].cell_32[i].key[0] == 0) && (arr_heavy[index_h].cell_32[i].key[1] == 0))
            {
                arr_heavy[index_h].cell_32[i].key[0] = *key;
                arr_heavy[index_h].cell_32[i].key[1] = *(key + 1);
                arr_heavy[index_h].cell_32[i].count += fint;
                return flog_false;
            }
            else
            {
                if(arr_heavy[index_h].cell_32[i].count < arr_heavy[index_h].cell_32[min_num32].count)
                    min_num32 = i;

                if(i == num_32-1)
                {
                    hc_min = arr_heavy[index_h].cell_32[min_num32].count;
                }
            }
    }  

    for (int i = 0; i < num_16; ++i)
    {
        //key[0] is the source ID，key[1] is the destination ID, the "stoi()" function converts a string to an int, determine whether to convert to "uint32"
        if ((arr_heavy[index_h].cell_16[i].key[0] == *key && (arr_heavy[index_h].cell_16[i].key[1] == *(key + 1))))  
        {
            if(arr_heavy[index_h].cell_16[i].count+fint <= 65535) arr_heavy[index_h].cell_16[i].count += fint;
            else 
            {
                
                for (int j = 0; j < num_32; ++j)
                {
                    if(arr_heavy[index_h].cell_32[j].count <= 65535) //
                    {
                        key_temp[0] = arr_heavy[index_h].cell_32[j].key[0];
                        key_temp[1] = arr_heavy[index_h].cell_32[j].key[1];
                        weight_temp = arr_heavy[index_h].cell_32[j].count;

                        arr_heavy[index_h].cell_32[j].key[0] = arr_heavy[index_h].cell_16[i].key[0];
                        arr_heavy[index_h].cell_32[j].key[1] = arr_heavy[index_h].cell_16[i].key[1];
                        arr_heavy[index_h].cell_32[j].count = arr_heavy[index_h].cell_16[i].count + fint;

                        arr_heavy[index_h].cell_16[i].key[0] = key_temp[0];
                        arr_heavy[index_h].cell_16[i].key[1] = key_temp[1];
                        arr_heavy[index_h].cell_16[i].count = weight_temp;

                        return flog_false;
                    }

                    if(j == num_32-1)
                        full_32++;
                        //cout << "cell of 32bit is not enough" <<endl;
                }
            }
            return flog_false;
        } 
        else if ((arr_heavy[index_h].cell_16[i].key[0] == 0) && (arr_heavy[index_h].cell_16[i].key[1] == 0))
        {
            arr_heavy[index_h].cell_16[i].key[0] = *key;
            arr_heavy[index_h].cell_16[i].key[1] = *(key + 1);
            arr_heavy[index_h].cell_16[i].count += fint;
            return flog_false;
        }
        else
        {
            if(arr_heavy[index_h].cell_16[i].count < arr_heavy[index_h].cell_16[min_num16].count)
                min_num16 = i;

            if(i == num_16-1)
            {
                //hc_min = arr_heavy[index_h].cell_16[min_num16].count;  //Get the minimum edge weights in the cell
                flog_false = true;  //Insertion failure
                if(hc_min < arr_heavy[index_h].cell_16[min_num16].count)
                {
                    //hc_min = arr_heavy[index_h].cell_32[min_num32].count;  //Get the minimum edge weights in the cell

                    //Swap the 32bit minimum with the 16bit minimum cell
                    key_temp[0] = arr_heavy[index_h].cell_32[min_num32].key[0];
                    key_temp[1] = arr_heavy[index_h].cell_32[min_num32].key[1];
                    weight_temp = arr_heavy[index_h].cell_32[min_num32].count;

                    arr_heavy[index_h].cell_32[min_num32].key[0] = arr_heavy[index_h].cell_16[min_num16].key[0];
                    arr_heavy[index_h].cell_32[min_num32].key[1] = arr_heavy[index_h].cell_16[min_num16].key[1];
                    arr_heavy[index_h].cell_32[min_num32].count = arr_heavy[index_h].cell_16[min_num16].count;

                    arr_heavy[index_h].cell_16[min_num16].key[0] = key_temp[0];
                    arr_heavy[index_h].cell_16[min_num16].key[1] = key_temp[1];
                    arr_heavy[index_h].cell_16[min_num16].count = weight_temp;

                }
                hc_min = arr_heavy[index_h].cell_16[min_num16].count; //Get the minimum edge weights in the cell
                min_num = min_num16;
                
                return flog_false;
            }
        }
    }  

    return flog_false;
}

//Query for weight of edge
uint32_t SIM_heavy::H_EdgeQuery(unsigned int index_h, uint32_t * key)
{
    //bool quary_flog = false;
    for (int i = 0; i < num_16; ++i)
    {
        //key[0] is the source ID，key[1] is the destination ID, the "stoi()" function converts a string to an int, determine whether to convert to "uint32"
        if ((arr_heavy[index_h].cell_16[i].key[0] == *key && (arr_heavy[index_h].cell_16[i].key[1] == *(key + 1))))  
        {
            return arr_heavy[index_h].cell_16[i].count;
        } 
    }

    for (int i = 0; i < num_32; ++i)
    {
        //key[0] is the source ID，key[1] is the destination ID, the "stoi()" function converts a string to an int, determine whether to convert to "uint32"
        if ((arr_heavy[index_h].cell_32[i].key[0] == *key && (arr_heavy[index_h].cell_32[i].key[1] == *(key + 1))))  
        {
            return arr_heavy[index_h].cell_32[i].count;
        } 
    }
    return 0;    
}

//Query for weight of node
uint32_t SIM_heavy::H_NodeQuery(unsigned int index_h, uint32_t * key)
{
    uint32_t H_result = 0;

    for(int i = 0; i < length; i++)
    {       
        //lterate all the cell
        for (int j = 0; j < num_16; ++j)
        {
            if (arr_heavy[index_h+i].cell_16[j].key[0] == *key)  //key[0] is scr ID，
            {
                H_result += arr_heavy[index_h+i].cell_16[j].count;
            } 
        }
        for (int j = 0; j < num_32; ++j)
        {
            if (arr_heavy[index_h+i].cell_32[j].key[0] == *key) 
            {
                H_result += arr_heavy[index_h+i].cell_32[j].count;
            } 
        }
    }

    return H_result;    
}


//Query for Heavy Hitter edge
void SIM_heavy::H_HH_Edgequery(val_tp thresh, myvector& HH_Edge)
{
    uint32_t empty_count = 0;
    uint32_t empty_32bit_count = 0;
    uint32_t HH_Count = 0;
    std::pair<key_tp, val_tp> edge_temp;
    //only query in the heavy part
    for(int i = 0; i < length * width; i++)
    {
        for(int j = 0; j < num_16; j++)
        {
            if(arr_heavy[i].cell_16[j].count == 0)
                empty_count++;

            if(arr_heavy[i].cell_16[j].count > thresh)
            {
                edge_temp.first.key[0] = arr_heavy[i].cell_16[j].key[0];
                edge_temp.first.key[1] = arr_heavy[i].cell_16[j].key[1];
                edge_temp.second = arr_heavy[i].cell_16[j].count;
                HH_Edge.push_back(edge_temp);
                HH_Count++;
            }
        }

        for(int j = 0; j < num_32; j++)
        {
            if(arr_heavy[i].cell_32[j].count == 0)
            {
                empty_count++;
                empty_32bit_count++;
            }

            if(arr_heavy[i].cell_32[j].count > thresh)
            {
                edge_temp.first.key[0] = arr_heavy[i].cell_32[j].key[0];
                edge_temp.first.key[1] = arr_heavy[i].cell_32[j].key[1];
                edge_temp.second = arr_heavy[i].cell_32[j].count;
                HH_Edge.push_back(edge_temp);
                HH_Count++;
            }
        }
        
    }
    cout << "empty counter is " << empty_count <<endl;
    cout << "empty 32bit counter is " << empty_32bit_count <<endl;
    cout << "query for HH number is " << HH_Count <<endl;
}


//Query for Heavy Hitter node（all heavy nodes）
void SIM_heavy::H_HH_Nodequery(myset& candidate_node)
{
    
    //key_tp reskey;

    for(int i = 0; i < length * width; i++)
    {
        key_tp reskey;
        for(int j = 0; j < num_16; j++)
        {                     
            memcpy(reskey.key, arr_heavy[i].cell_16[j].key, LGN/2);
            candidate_node.insert(reskey);
            memcpy(reskey.key, arr_heavy[i].cell_16[j].key+1, LGN/2);
            candidate_node.insert(reskey);
        } 
        for(int j = 0; j < num_32; j++)
        {          
            memcpy(reskey.key, arr_heavy[i].cell_32[j].key, LGN/2);
            candidate_node.insert(reskey);
            memcpy(reskey.key, arr_heavy[i].cell_32[j].key+1, LGN/2);
            candidate_node.insert(reskey);
        }
    }
}


//Query for Heavy Hitter node (nodes with an entire row weight greater than a threshold)
void SIM_heavy::H_HH_Nodequery(val_tp thresh, myset& candidate_node)
{
    key_tp reskey;

    for(int k = 0; k < width; k++)
    {
        val_tp rowsum = 0;
        for(int i = 0; i < length; i++)
        {
            for(int j = 0; j < num_16; j++)
            {          
                rowsum += this->arr_heavy[i+k*length].cell_16[j].count;
            } 
            for(int j = 0; j < num_32; j++)
            {          
                rowsum += this->arr_heavy[i+k*length].cell_32[j].count;
            } 
        }
        if(rowsum > thresh)
        {
            for(int i = 0; i < length; i++)
            {
                key_tp reskey;
                for(int j = 0; j < num_16; j++)
                {           
                    memcpy(reskey.key, arr_heavy[i].cell_16[j].key, LGN/2);
                    candidate_node.insert(reskey);
                } 
                for(int j = 0; j < num_32; j++)
                {           
                    memcpy(reskey.key, arr_heavy[i].cell_32[j].key, LGN/2);
                    candidate_node.insert(reskey);
                } 
            }
        }
    }
}


void SIM_heavy::Reset()
{
    hc_min = 0;
    min_num = 0;
    flog_false = false;

    //Initialize the SIM data structure to 0.
    memset(arr_heavy, 0, sizeof(Bucket) * length * width); 
    
}


void SIM_heavy::heavay_print(myvector& HH_Edge_16, myvector& HH_Edge_32)
{
    std::pair<key_tp, val_tp> edge_temp;
    for(int i = 0; i < length * width; i++)
    {
        for(int j = 0; j < num_16; j++)
        {
            edge_temp.first.key[0] = arr_heavy[i].cell_16[j].key[0];
            edge_temp.first.key[1] = arr_heavy[i].cell_16[j].key[1];
            edge_temp.second = arr_heavy[i].cell_16[j].count;
            HH_Edge_16.push_back(edge_temp);
        }
    }

    for(int i = 0; i < length * width; i++)
    {
        for(int j = 0; j < num_32; j++)
        {

            edge_temp.first.key[0] = arr_heavy[i].cell_32[j].key[0];
            edge_temp.first.key[1] = arr_heavy[i].cell_32[j].key[1];
            edge_temp.second = arr_heavy[i].cell_32[j].count;
            HH_Edge_32.push_back(edge_temp);

        }
    }
    
}


#endif
