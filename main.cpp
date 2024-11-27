#include "main.hpp"
#include "adaptor.hpp"
#include <unordered_map>
#include <utility>
#include <iostream>
#include <fstream>
#include "util.h"
#include "datatypes.hpp"
#include <iomanip>
#include "RrConfig.h"
#include "SIM.hpp"

using namespace std;

int main(int argc, char* argv[]) 
{
    rr::RrConfig config;
    
    ofstream outfile("result.txt");
    //Reading configuration files
    if(config.ReadConfig("config_DBLP.ini") == false) 
    {
        std::cout << "Error loading configuration file." << std::endl;
        return 1;
    }

    int num = 1;
    for(;;) 
    {
        //Prints the name of the "map" and the configuration of the parameters
        std::string dm_config = "SIM_" + std::to_string(num);  //"to_string()"" converted to a string variable。
        if(config.ReadString(dm_config.c_str(), "name", "") == "") 
        {
            std::cout << "End at the " << num << "-th configuration " << dm_config << "." << std::endl;
            return 1;
        }
        std::cout << config.ReadString(dm_config.c_str(), "name", "") << ":" << std::endl;   //Read the value of "name" and print the name of the dataset
        num++;

        //Dataset list
        const char* filenames = config.ReadString(dm_config.c_str(), "filenames", "").c_str();   //Read the value of "filename"
        unsigned long long buf_size = 5000000000;  //5 billion bytes

        //Heavy-hitter, heavy-changer and reachability threshold
        double alpha = config.ReadFloat(dm_config.c_str(), "alpha", 0);   //Read the value of "filename"
        double alpha2 = config.ReadFloat(dm_config.c_str(), "alpha2", 0);
        double beta = config.ReadFloat(dm_config.c_str(), "beta", 0); 
        double beta2 = config.ReadFloat(dm_config.c_str(), "beta2", 0);
        double gamma = config.ReadFloat(dm_config.c_str(), "gamma", 0);
        std::cout << "alpha = " << alpha << "； alpha2 = " << alpha2 << ";"<<endl;
        cout<< "beta = " << beta << "; beta2 = " << beta2 << "; gamma = " << gamma << ";" << std::endl;   //Set the threshold for output

        //"SIM parameters" reads the parameter Settings of the data structure
        int H_length = config.ReadInt(dm_config.c_str(), "length1", 0);
        int H_width = config.ReadInt(dm_config.c_str(), "width1", 0);
        int L_length = config.ReadInt(dm_config.c_str(), "length2", 0);
        int L_width = config.ReadInt(dm_config.c_str(), "width2", 0);
        cout << "Heavy part: " <<endl;
        std::cout << "length * width: " << H_length << " * " << H_width << std::endl;
        cout << "Light part: " <<endl;
        std::cout << "length * width: " << L_length << " * " << L_width << std::endl;

        outfile << "Heavy part: " << "length * width: " << H_length << " * " << H_width << std::endl;
        outfile << "Light part: " << "length * width: " << L_length << " * " << L_width << std::endl;

        int nodenum = config.ReadInt(dm_config.c_str(), "nodenum", 0);  //Read the number of nodes

        int numfile = 0;
        
        //Some metrics are defined such as relative error, average relative error
        // Average relative error for weight-based query
        double reedge = 0, renode = 0, resubgraph = 0;
        double areedge = 0, arenode = 0, aresubgraph = 0;

        // Precision, recall for rechability query
        double prerechability = 0, avrprerechability = 0, rerechability = 0, avrrerechability = 0;
    
        // Precision, recall, average relative error for heavy-key query

        // Heavy-hitter edge
        double prehitteredge=0, rehitteredge=0, reerhitteredge=0;
        double avrprehitteredge=0, avrrehitteredge=0, avrreerhitteredge=0;

        // Heavy-hitter node
        double prehitternode=0, rehitternode=0, reerhitternode=0;
        double avrprehitternode=0, avrrehitternode=0, avrreerhitternode=0;

        // Time cost
        double updatetime=0, avrupdatetime=0, querytime1=0, avrquerytime1=0, querytime2=0, avrquerytime2=0, querytime3=0, avrquerytime3=0;
        double querytime4=0, avrquerytime4=0, querytime5=0, avrquerytime5=0, querytime6=0, avrquerytime6=0;

        //Reading the dataset
        std::string file;

        std::ifstream tracefiles(filenames);  //filenames = dblpfiles;
        if (!tracefiles.is_open()) 
        {
            std::cout << "Error opening file" << std::endl;
            return -1;
        }

        //Instantiation of the hash function map
        groundmap groundedgetmp, groundnodetmp;
        mymap groundedge, groundnode;
        tuple_t t;  //A struct representing the input data in the form "key(source id, destination id) + size"
        val_tp diffsum = 0, diffsum_node = 0;

        myvector heavyhitteredge, heavyhitternode, oldheavyhitteredge, oldheavyhitternode, alledge, paths;

        //SIM
        //SIM sim(H_length, H_width, L_length, L_width);

        HeavyChanger<SIM>* HC_Sim = new HeavyChanger<SIM>(H_length, H_width, L_length, L_width);
        
        //Iterate over each dataset file
        for (std::string file; getline(tracefiles, file);) 
        {   
            //Load traces and get ground. That is, one loop loads one line of "ablpfiles" and processes one data set file.

            Adaptor* adaptor =  new Adaptor(file, buf_size);  //"file" represents a row in the file and is stored in "buf_size".
            std::cout << "[Dataset]: " << file << std::endl;

            outfile << "[Dataset]: " << file << std::endl;

            //Get ground
            adaptor->Reset();  //Resets the "adaptor" to its initial state.

            //At the beginning of the new measurement period, the old and new edge weights are updated.
            //Reset gounrdtmp;   "groundedgetmp" is an unordered map container.   
            //The edge weights from the previous time period are put into second[1], and in the new period, second[0] is used to record the edge weights.
            for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) {
                it->second[1] = it->second[0];
                it->second[0] = 0;
            }

            for (auto it = groundnodetmp.begin(); it != groundnodetmp.end(); it++) {
                it->second[1] = it->second[0];
                it->second[0] = 0;
            }

            //Insert into ground table，store edges and edge weights
            val_tp sum = 0;  //The total edge weights are recorded
            memset(&t, 0, sizeof(tuple_t));  //Set "t" to zero, where "t" represents the custom input data variable
            
            //Record the edges in the dataset to "ground" (both edges and nodes)
            while(adaptor->GetNext(&t) == 1) 
            {
                sum += t.size;
                key_tp key; //A struct of two 32-bit arrays
                memcpy(key.key, &(t.key), LGN);  //"key.key" represents the property key[2] in the "key" struct and is used to store the addresses of "src and des" in the dataset
                if (groundedgetmp.find(key) != groundedgetmp.end()) {
                    groundedgetmp[key][0] += t.size; //The "map" container overloads the "[]", you can directly index the pairs in the "map", and determine whether there is a "key" by the "IF" statement. If there is a key, "value+size" (edge weight)       
                } else {  //The edge does not exist in "groundedgetmp", and the ID and weight of the edge are re-recorded.
                    val_tp* valtuple = new val_tp[2]();
                    groundedgetmp[key] = valtuple;   //
                    groundedgetmp[key][0] += t.size;
                }

                key_tp keynode;
                // Src nodes，the source node and the edge weight of this node are stored in "groundnodetmp".
                memcpy(keynode.key, &(t.key.src_ip), LGN/2);
                if (groundnodetmp.find(keynode) != groundnodetmp.end()) {
                    groundnodetmp[keynode][0] += t.size;//"+size" is the weight of the edge in which the node is located.
                } else {
                    val_tp* valtuple = new val_tp[2]();
                    groundnodetmp[keynode] = valtuple;
                    groundnodetmp[keynode][0] += t.size;
                }
            }
            
            std::pair<key_tp, val_tp> edge_temp;
            //Record all edges in an "alledge" container
            for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) 
            {
                edge_temp.first.key[0] = it->first.key[0];
                edge_temp.first.key[1] = it->first.key[1];
                edge_temp.second = it->second[0];
                alledge.push_back(edge_temp);
            } 

            uint64_t t1, t2;
            adaptor->Reset();   //Adapter into the initial state.

            //Initialize the data structure
            //sim.Reset();
            HC_Sim->Reset();        //Replace the old data structure with the contents of the new data structure and set the new data structure to zero.

            SIM* cursim = (SIM*)HC_Sim->GetCurSim();

            t1 = now_us();  //Record the current time
            while(adaptor->GetNext(&t) == 1) 
            {
                key_tp key;
                memcpy(key.key, &(t.key), LGN);
                cursim->insert(key.key, (val_tp)t.size);
            }
            t2 = now_us();  //Record end time
            cout <<"heavy溢出流大小：" << cursim->heavy.full_32<<endl;
            cout <<"light溢出流大小：" << cursim->light.light_full <<endl;
            updatetime = (double)(t2-t1)/1000000000;  //1000 clock cycles per second.
            avrupdatetime += updatetime;


            // Query for edge weight
            val_tp edgeerror = 0, edgecount = 0;
            t1 = now_us();
            for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) 
            {
                if (it->second[0] > 0){
                edgecount++;  //Record the number of edges
                val_tp esedgeweight = cursim->EdgeQuery((uint32_t*)(*it).first.key);
                val_tp error = esedgeweight > it->second[0] ? esedgeweight - it->second[0] : it->second[0] - esedgeweight;   // Get the absolute error
                edgeerror += error*1.0/it->second[0];  //Calculate relative error
                }
            }
            t2 = now_us(); 
            querytime1 = (double)(t2-t1)/(1000000000*edgecount);  //The average measurement time for each edge in this measurement period.
            avrquerytime1 += querytime1;
            reedge = edgeerror*1.0/edgecount;  //The average error of each edge in this measurement period.
            areedge += reedge;


            // Query for node weight (src)
            val_tp nodeerror = 0, nodecount = 0;
            t1 = now_us();
            for (auto it = groundnodetmp.begin(); it != groundnodetmp.end(); it++) 
            {
                if (it->second[0] > 0)
                {  //Check if the counter is empty to see if there are any records in the bucket
                    nodecount++;
                    val_tp esnodeweight = cursim->NodeQuery((uint32_t*)(*it).first.key);
                    val_tp error = esnodeweight > it->second[0] ? esnodeweight - it->second[0] : it->second[0] - esnodeweight;
                    nodeerror += error*1.0/it->second[0];
                }
            }
            t2 = now_us();
            querytime2 = (double)(t2-t1)/(1000000000*nodecount);
            avrquerytime2 += querytime2;
            renode = nodeerror*1.0/nodecount;
            arenode += renode;


            // Qurey for subgraph weight (10 nodes)
            //int nodenum = 10, subgraphnode = 0;  Construct the nodes in the subgraph.
            int subgraphnode = 0;
            key_tp nodekey[nodenum];  //Create a subgraph with ten nodes
            for (auto it = groundnodetmp.begin(); it != groundnodetmp.end(); it++) 
            {
                if (subgraphnode == nodenum) {break;}
                if (it->second[0] > 0)
                {
                    memcpy(nodekey[subgraphnode].key, it->first.key, LGN/2);  //Store nodes with weights in "groundnodetmp" into "nodekey" to get subgraph nodes.
                    subgraphnode++;
                }
            }
            val_tp essubgraph = 0, realsubgraph = 0;
            key_tp edgekey;
            //To start a subgraph query, we need to query the weight of the edges that each node forms with other nodes in the subgraph, which is 45 edges in total.
            t1 = now_us();
            for (int i = 0; i<nodenum-1; i++)
            {
                for (int j=i+1; j<nodenum; j++)
                {
                    memcpy(edgekey.key, nodekey[i].key, LGN/2);
                    memcpy(edgekey.key+1, nodekey[j].key, LGN/2);  //Get the ID of the edges in the subgraph

                    essubgraph += cursim->EdgeQuery((uint32_t*)edgekey.key);  //Edge weight query, get the weight of edges in the subgraph.

                }
            }
            t2 = now_us();
            querytime3 = (double)(t2-t1)/1000000000;
            avrquerytime3 += querytime3;
            
            for (int i = 0; i<nodenum-1; i++)
            {
                for (int j=i+1; j<nodenum; j++)
                {
                    memcpy(edgekey.key, nodekey[i].key, LGN/2);
                    memcpy(edgekey.key+1, nodekey[j].key, LGN/2);  //Get the ID of the edges in the subgraph

                    //Query the edges of the true subgraph
                    for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) 
                    {
                        if (memcmp(it->first.key, edgekey.key, LGN) == 0) 
                        {
                            realsubgraph += it->second[0];  //Find the same edge as in the subgraph in "groundedgetmp" and record its weight to get the true subgraph weight.
                        }
                    }
                }
            }

            if (realsubgraph != 0)
            {
                resubgraph = (essubgraph - realsubgraph)*1.0/realsubgraph;
                cout << "realsubgraph=0" << endl;
            } 
            else {resubgraph = essubgraph - realsubgraph;}   
            aresubgraph += resubgraph;    


            //Qurey for heavy hitter edge
            val_tp thresholdhitter = alpha * sum;
            t1 = now_us();
            cursim->heavy.H_HH_Edgequery(thresholdhitter, heavyhitteredge);
            t2 = now_us();
            querytime4 = (double)(t2-t1)/1000000000;
            avrquerytime4 += querytime4;

            int tp = 0, cnt = 0, fp = 0, sp = 0;
            reerhitteredge = 0;
            for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) 
            {
                if (it->second[0] > thresholdhitter) 
                {
                    cnt++;
                    for (auto res = heavyhitteredge.begin(); res != heavyhitteredge.end(); res++) 
                    {
                        if (memcmp(it->first.key, res->first.key, LGN) == 0) 
                        {
                            reerhitteredge += abs(long(res->second - it->second[0]))*1.0/it->second[0];
                            tp++;
                        }
                    }
                }
            }

            if (heavyhitteredge.size()==0) { prehitteredge = 1;}
            else { prehitteredge = tp*1.0/heavyhitteredge.size();}
            if (cnt==0) {rehitteredge = 1;}
            else {rehitteredge = tp*1.0/cnt;}
            if (tp==0) {reerhitteredge = 0;}
            else {reerhitteredge = reerhitteredge/tp;}
            avrprehitteredge += prehitteredge;
            avrrehitteredge += rehitteredge;
            avrreerhitteredge += reerhitteredge;


            //Qurey for heavy hitter node
            val_tp thresholdhitternode = alpha2 * sum;
            t1 = now_us();
            //sim.heavy.H_HH_Nodequery(thresholdhitter, heavyhitternode);
            cursim->HH_Nodequery(thresholdhitternode+1000, heavyhitternode);
            t2 = now_us();
            querytime5 = (double)(t2-t1)/1000000000;
            avrquerytime5 += querytime5;

            tp = 0;
            cnt = 0;
            reerhitternode = 0;
            for (auto it = groundnodetmp.begin(); it != groundnodetmp.end(); it++) {
                if (it->second[0] > thresholdhitternode) {
                    cnt++;
                    for (auto res = heavyhitternode.begin(); res != heavyhitternode.end(); res++) {
                        if (memcmp(it->first.key, res->first.key, LGN/2) == 0) {
                            reerhitternode += abs(long(res->second - it->second[0]))*1.0/it->second[0];
                            tp++;
                        }
                    }
                }
            }

            if (heavyhitternode.size() == 0){ prehitternode = 1;}
            else {prehitternode = tp*1.0/heavyhitternode.size();}
            if (cnt == 0) { rehitternode = 1; }
            else { rehitternode = tp*1.0/cnt;}
            if (tp == 0){ reerhitternode = 0;}
            else { reerhitternode = reerhitternode/tp;}
            avrprehitternode += prehitternode;
            avrrehitternode += rehitternode;
            avrreerhitternode += reerhitternode;


            // Query for path reachability (use nodes in subgraph query). Check if 45 edge paths are reachable.
            tp = 0;
            fp = 0;
            cnt = 0;
            t1 = now_us();
            for (int i = 0; i<nodenum-1; i++)
            {
                for (int j=i+1; j<nodenum; j++)
                {
                    memcpy(edgekey.key, nodekey[i].key, LGN/2);
                    memcpy(edgekey.key+1, nodekey[j].key, LGN/2);//"edgekey" stores the two nodes that need to be queried.

                    //cout << edgekey.key[0] <<" "<< edgekey.key[1] <<endl;

                    //Estimation results
                    if(cursim->BFS((uint32_t*)edgekey.key, gamma*sum))
                    {                                                 

                        sp++;  //Query the number of reachable that we get
                        
                        if(BFS((uint32_t*)edgekey.key, paths))
                        {
                            tp++;
                        }
                        fp = sp-tp;                       
                    }
                    
                    //Real results
                    if(BFS((uint32_t*)edgekey.key, paths))
                    {
                        cnt++;
                    }

		        }
            }
            
            t2 = now_us();
            int querynum = nodenum * (nodenum - 1)/2;
            querytime6 = (double)(t2-t1)/(1000000000*querynum);
            avrquerytime6 += querytime6;
            if (tp == tp + fp){prerechability = 1;} 
            else {prerechability = tp*1.0/(tp + fp);}
            if (tp == cnt){rerechability = 1;}
            else {rerechability = tp*1.0/cnt;}
            avrprerechability += prerechability;
            avrrerechability += rerechability;

            heavyhitteredge.clear();
            heavyhitternode.clear();

            numfile++;
            delete adaptor;
        }


        //Delete
        for (auto it = groundedgetmp.begin(); it != groundedgetmp.end(); it++) 
        {
            if(it->second !=NULL)
            {
                delete [] it->second;
                it->second = NULL;
            }
            
        }
        delete HC_Sim;

        //std::cout << "-----------------------------------------------   Summary    -------------------------------------------------------" << std::endl;

        //base weight
        std::cout << std::setw(30) << std::left << "ARError. edge weight: " << areedge/numfile <<  "     ARError. node weight: " << arenode/numfile 
        << "     ARError. subgraph: " << aresubgraph/numfile
        << std:: endl;


        //heavy hitter edge/node
        std::cout << std::setw(30)<< std::left << "APR. heavy-hitter edge" << std::setw(30) << std::left << "ARE. heavy-hitter edge"
        << std::setw(30) <<  "ARError. heavy-hitter edge" << std::endl;
        std::cout << std::setw(30)<< std::left << avrprehitteredge/numfile << std::setw(30) << std::left << avrrehitteredge/numfile
        << std::setw(30) <<  avrreerhitteredge/numfile << std::endl;

        std::cout << std::setw(30)<< std::left << "APR. heavy-hitter node" << std::setw(30) << std::left << "ARE. heavy-hitter node"
        << std::setw(30) <<  "ARError. heavy-hitter node" << std::endl;
        std::cout << std::setw(30)<< std::left << avrprehitternode/numfile << std::setw(30) << std::left << avrrehitternode/numfile
        << std::setw(30) <<  avrreerhitternode/numfile << std::endl;


        //path
        std::cout << std::setw(30)<< std::left << "APR. path reachability" << std::setw(30) << std::left << "ARE. path reachability" << std::endl;
        std::cout << std::setw(30)<< std::left << avrprerechability/numfile << std::setw(30) << std::left << avrrerechability/numfile << std::endl;
        
        //time
        std::cout << std::setw(30) << std::left << "AvrT. Update: " << avrupdatetime/numfile << std::endl;
        //<< std::setw(30) << std::left << "AvrT. node weight" << std::endl;
        std::cout << std::setw(30) << std::left << "AvrT. edge weight: " << avrquerytime1/numfile 
        << "     AvrT. node weight: " << avrquerytime2/numfile  
        << "     AvrT. subgraph(10 nodes): " << avrquerytime3/numfile
        << std::endl;

        std::cout << "AvrT. heavy-hitter edge: " << avrquerytime4/numfile << "    AvrT. heavy-hitter node: " << avrquerytime5/numfile  << std::endl;
        
        std::cout << "AvrT. reachability: " << avrquerytime6/numfile << std::endl;
        
        
        //Output to file
        outfile << "ARError. edge weight: " << areedge/numfile << "     ARError. node weight: " << arenode/numfile << "     ARError. subgraph: " << aresubgraph/numfile << std:: endl; 
        outfile << std::setw(30)<< std::left << "APR. heavy-hitter edge" << std::setw(30) << std::left << "ARE. heavy-hitter edge"
        << std::setw(30) <<  "ARError. heavy-hitter edge" << std::endl;
        outfile << std::setw(30)<< std::left << avrprehitteredge/numfile << std::setw(30) << std::left << avrrehitteredge/numfile
        << std::setw(30) <<  avrreerhitteredge/numfile << std::endl;
        
        outfile << std::setw(30)<< std::left << "APR. heavy-hitter node" << std::setw(30) << std::left << "ARE. heavy-hitter node"
        << std::setw(30) <<  "ARError. heavy-hitter node" << std::endl;
        outfile << std::setw(30)<< std::left << avrprehitternode/numfile << std::setw(30) << std::left << avrrehitternode/numfile
        << std::setw(30) <<  avrreerhitternode/numfile << std::endl;

        outfile << std::setw(30)<< std::left << "APR. path reachability" << std::setw(30) << std::left << "ARE. path reachability" << std::endl;
        outfile << std::setw(30)<< std::left << avrprerechability/numfile << std::setw(30) << std::left << avrrerechability/numfile << std::endl;

        outfile << "AvrT. edge weight: " << avrquerytime1/numfile << "     AvrT. node weight: " << avrquerytime2/numfile  << "     AvrT. subgraph(10 nodes): " << avrquerytime3/numfile << std::endl;
        outfile << "AvrT. heavy-hitter edge: " << avrquerytime4/numfile << "    AvrT. heavy-hitter node: " << avrquerytime5/numfile  << std::endl;
        outfile << "AvrT. reachability: " << avrquerytime6/numfile << std::endl;

        outfile << "AvrT. Update: " << avrupdatetime/numfile << std::endl;
        outfile << "-----------------------------------------------   end    -------------------------------------------------------" << std::endl;
    }
    outfile.close();
}
