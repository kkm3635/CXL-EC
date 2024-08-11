#include <bits/stdc++.h>
#include <cstdint>
#include <limits>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <functional>
#include <cstdlib>
#include <ctime>
#include <cstdio>


#define CGSIZE 4
#define PARITY 2

using namespace std;

class ConsistentHashRing {
private:
	map<int, float> ring; // hash value와 node mapping
	int virtual_node;
    int* assigned_devices;
	uint32_t entire_node_num;

public:
	ConsistentHashRing(int virtual_node = 1) : virtual_node(virtual_node) {
        assigned_devices = new int[CGSIZE+PARITY];  
    }

	// Function to add Node in the ring
	void add_node(const int node_num) {
		entire_node_num = node_num;
		for(int i = 0; i < node_num; ++i){
			ring[i] = static_cast<double>(i)/entire_node_num;
			//printf("%f\n",ring[i]);
		}
	}

	// // Function to remove Node from the ring
	// void remove_node(const uint32_t& device) {
	// 	for (int i = 0; i < virtual_node; ++i) {
	// 		unsigned int replica_key = get_hash(device);
	// 		ring.erase(replica_key);
	// 		sorted_keys.erase(replica_key);
	// 	}
	// }

	int* get_node(const float& CGgroup_ID) {
		int start_dev;

		if (ring.empty()) {
			return 0;
		}
		float hash_value = CGgroup_ID;
		//printf("hash value: %f\n",hash_value);

		if(hash_value > 0.937500){
			start_dev = 0;
		}
		else{
			for(const auto& pair : ring){ // hash value로 가장 가까운 device 찾는거
				if(pair.second >= hash_value){
					start_dev = pair.first;
					//std::cout << start_dev << std::endl;
					break;
				}
			}	
		}

		for(int i = 0; i < CGSIZE+PARITY; ++i){
			assigned_devices[i] = start_dev;
			start_dev++;
			if(start_dev == 16) start_dev =0;

        	//printf("%d, ", assigned_devices[i]);
    	}
    	//printf("\n=============\n");

		return assigned_devices;
	}

	int* get_node_random(const uint32_t& CGgroup_ID) {
		if(ring.empty()) {
			return 0;
		}
		for (int i = 0; i < CGSIZE+PARITY; ++i) {
        	assigned_devices[i] = rand() % (entire_node_num);
        	for (int j = 0; j < i; ++j) {
            	if (assigned_devices[i] == assigned_devices[j]) {
                // 중복된 값이 발견되면 현재 인덱스 다시 생성
                	--i;
                	break;
				}
			}
		}
			
		return assigned_devices;
	}
};