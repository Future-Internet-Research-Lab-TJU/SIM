#ifndef CHANGER_H
#define CHANGER_H

#include "SIM.hpp"
#include "SIM_heavy.hpp"
#include "datatypes.hpp"

// The template parameters are, in order, the key type, the value type, the hash function, the equal comparator, and the memory allocator
typedef std::unordered_map<key_tp, val_tp*, key_tp_hash, key_tp_eq> groundmap;

bool BFS(uint32_t* key, std::vector<std::pair<key_tp, val_tp> >& path)
{
    
    vector<uint32_t> res;//Store the source ID to be queried, that is, store the candidate root node
    queue <uint32_t> dstres;
    res.clear();
    uint32_t tem_res = 0;
    uint32_t* reskey = &tem_res;//The pointer stores the source ID
    memcpy(reskey, key, LGN/2);
    res.push_back(*reskey); //Put the first root node into the candidate container.
    dstres.push(*reskey);

    while (!dstres.empty())
    {
        uint32_t re = dstres.front();  //Take an element at the head of the line as the node to view
        if (memcmp(key+1, &re, LGN/2)==0){return true;}//Determine whether it is the destination node and whether it is reachable.
        for (auto it = path.begin(); it != path.end(); it++) //If the destination node is not reachable, the judged destination node is taken as the source node, and the relevant destination nodes are searched in the "path" heavy edge.
        {
            
            if (memcmp(&re, it->first.key, LGN/2)==0)
            {
                memcpy(reskey, it->first.key+1, LGN/2);
                if(find(res.begin(), res.end(), *reskey) == res.end())
                {
                    if (memcmp(key+1, reskey, LGN/2)==0){return true;}
                    res.push_back(*reskey);  //Put the candidate nodes into the container
                    dstres.push(*reskey); 
                }
                
            }
        }

        dstres.pop();        
    }
    return false;
}


template <class S>
class HeavyChanger 
{
private:
    S* old_sim;

    S* cur_sim;

    //int lgn_;

public:
    HeavyChanger(int hea_l, int hea_w, int lig_l, int lig_w);

    ~HeavyChanger();

    void insert(uint32_t * key, val_tp val);

    val_tp Change_sum();

    void Querychangeredge(val_tp thresh, myvector& result);

    val_tp Change_node_sum();

    void Querychangernode(val_tp thresh, myvector& result);

    void Changedhitteredge(myvector& oldresult, myvector& curresult); // The number of changed heavy hitters on edge between two adjacent epoch

    void Changedhitternode(myvector& oldresult, myvector& curresult, unsigned int oldnum, unsigned int curnum);

    void Reset();

    S* GetCurSim();

    S* GetOldSim();

};

template <class S>
HeavyChanger<S>::HeavyChanger(int hea_l, int hea_w, int lig_l, int lig_w) 
{
    old_sim = new S(hea_l, hea_w, lig_l, lig_w);
    cur_sim = new S(hea_l, hea_w, lig_l, lig_w);
}

template <class S>
HeavyChanger<S>::~HeavyChanger() 
{
    delete old_sim;
    delete cur_sim;
}

template <class S>
void HeavyChanger<S>::insert(uint32_t * key, val_tp val) 
{
    cur_sim->insert(key, val);
}

template <class S>
val_tp HeavyChanger<S>::Change_sum() 
{
    val_tp diff = 0;
    val_tp old_count, cur_count;

    //Take the change of the heavy edge
    for(int i = 0; i < cur_sim->l1 * cur_sim->w1; i++)
    {
        old_count = 0;
        cur_count = 0;
        for(int j = 0; j < num_16; j++)
        {
            cur_count += cur_sim->heavy.arr_heavy[i].cell_16[j].count;
            old_count += old_sim->heavy.arr_heavy[i].cell_16[j].count;
        }
        for(int j = 0; j < num_32; j++)
        {
            cur_count += cur_sim->heavy.arr_heavy[i].cell_32[j].count;
            old_count += old_sim->heavy.arr_heavy[i].cell_32[j].count;
        }
        diff += cur_count > old_count ? cur_count - old_count : old_count - cur_count;
    }


    //Take the change of the light edge
    for(int i = 0; i < cur_sim->l2 * cur_sim->w2; i++)
    {
        old_count = cur_sim->light.arr_light[i];
        cur_count = old_sim->light.arr_light[i];
        diff += cur_count > old_count ? cur_count - old_count : old_count - cur_count;
    }

    cout << "sim_diff = " << diff <<endl;
    return diff;

}

template <class S>
void HeavyChanger<S>::Querychangeredge(val_tp thresh, myvector& result) {
    myvector res1, res2;
    cur_sim->heavy.H_HH_Edgequery(thresh, res1);
    old_sim->heavy.H_HH_Edgequery(thresh, res2);
    myset reset;

    //Get the candidate change edges
    for (auto it = res1.begin(); it != res1.end(); it++) 
    {
        reset.insert(it->first);
    }
    for (auto it = res2.begin(); it != res2.end(); it++) 
    {
        reset.insert(it->first);
    }
    val_tp old_weight;
    val_tp new_weight;
    val_tp change;
//    val_tp maxchange = 0, sumchange = 0;
    for (auto it = reset.begin(); it != reset.end(); it++) 
    {
        old_weight = old_sim->EdgeQuery((uint32_t*)(*it).key);

        new_weight = cur_sim->EdgeQuery((uint32_t*)(*it).key);

        change = new_weight > old_weight ? new_weight - old_weight : old_weight - new_weight;
        
        if (change > thresh) 
        {
            key_tp key;
            memcpy(key.key, it->key, LGN);
            std::pair<key_tp, val_tp> cand;
            cand.first = key;
            cand.second = change;

            result.push_back(cand);
        }
    }
}

template <class S>
val_tp HeavyChanger<S>::Change_node_sum() 
{
    val_tp diff = 0;
    val_tp old_rowsum, cur_rowsum;

    
    //Take the change of the heavy edge
    for(int k = 0; k < cur_sim->w1; k++)
    {
        old_rowsum = 0;
        cur_rowsum = 0;
        for(int i = 0; i < cur_sim->l1; i++)
        {           
            for(int j = 0; j < num_16; j++)
            {
                cur_rowsum += cur_sim->heavy.arr_heavy[i+k*cur_sim->l1].cell_16[j].count;
                old_rowsum += old_sim->heavy.arr_heavy[i+k*old_sim->l1].cell_16[j].count;
            }
            for(int j = 0; j < num_32; j++)
            {
                cur_rowsum += cur_sim->heavy.arr_heavy[i+k*cur_sim->l1].cell_32[j].count;
                old_rowsum += old_sim->heavy.arr_heavy[i+k*old_sim->l1].cell_32[j].count;
            }
            
        }
        diff += cur_rowsum > old_rowsum ? cur_rowsum - old_rowsum : old_rowsum - cur_rowsum;
    }


    //Take the change of the light edge
    for(int i = 0; i < cur_sim->w2; i++)
    {
        old_rowsum = 0;
        cur_rowsum = 0;
        for(int j = 0; j < cur_sim->l2; j++)
        {
            old_rowsum += cur_sim->light.arr_light[i];
            cur_rowsum += old_sim->light.arr_light[i];            
        }
        diff += cur_rowsum > old_rowsum ? cur_rowsum - old_rowsum : old_rowsum - cur_rowsum;
    }

    cout << "sim_diff_node = " << diff <<endl;
    return diff;

}

//Query for heavy changer on node
template <class S>
void HeavyChanger<S>::Querychangernode(val_tp thresh, myvector& result) 
{

    myvector res1, res2;
    //Use the threshold of changer to find out the heavy node, which is the candidate node
    cur_sim->HH_Nodequery(thresh, res1);
    old_sim->HH_Nodequery(thresh, res2);
    myset reset;
    for (auto it = res1.begin(); it != res1.end(); it++) 
    {
	    reset.insert(it->first);
    }
    for (auto it = res2.begin(); it != res2.end(); it++) 
    {
        reset.insert(it->first);
    }
    
    //Check if the candidate node is a changer node
    val_tp old_node_weight;
    val_tp new_node_weight;
    val_tp change_node;
    for (auto it = reset.begin(); it != reset.end(); it++) 
    {
        old_node_weight = old_sim->NodeQuery((uint32_t*)(*it).key);

        new_node_weight = cur_sim->NodeQuery((uint32_t*)(*it).key);

        change_node = new_node_weight > old_node_weight ? new_node_weight - old_node_weight : old_node_weight - new_node_weight;

        if (change_node > thresh) 
        {
            key_tp key;
            memcpy(key.key, it->key, LGN/2);
            std::pair<key_tp, val_tp> cand;
            cand.first = key;
            cand.second = change_node;

            result.push_back(cand);
        }
    }
}

template <class S>
void HeavyChanger<S>::Reset() 
{
    old_sim->Reset();
    S* temp = old_sim;
    old_sim = cur_sim;
    cur_sim = temp;
}

//Returns the current "Dmatrix"
template <class S>
S* HeavyChanger<S>::GetCurSim() 
{
    return cur_sim;
}

template <class S>
S* HeavyChanger<S>::GetOldSim() 
{
    return old_sim;
}


#endif
