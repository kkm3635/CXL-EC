#include <bits/stdc++.h>
#include <cstdint>
#include <limits>

#define CGSIZE 4
#define PARITY 2

using namespace std;

class ConsistentHashRing {
private:
	map<int, uint32_t> ring;
	set<uint32_t> sorted_keys;
	int virtual_node;
    int* assigned_devices;
	uint32_t entire_node_num;

	unsigned int get_hash(const uint32_t& value) {
		hash<uint32_t> hash_function;
		return hash_function(value);
	}

public:
	ConsistentHashRing(int virtual_node = 1) : virtual_node(virtual_node) {
        assigned_devices = new int[CGSIZE+PARITY];
    }

	// Function to add Node in the ring
	void add_node(const int node_num) {
		// for (int i = 0; i < virtual_node; ++i) {
		// 	uint32_t replica_key = get_hash(device);
		// 	//printf("%d node의 hash 값은 %d\n")
		// 	ring[replica_key] = device;
		// 	sorted_keys.insert(replica_key);
		// }
		entire_node_num = node_num;
		for(uint32_t i = 0; i < node_num; ++i){
			ring[i] = i;
			sorted_keys.insert(i);
		}
	}

	// Function to remove Node from the ring
	void remove_node(const uint32_t& device) {
		for (int i = 0; i < virtual_node; ++i) {
			unsigned int replica_key = get_hash(device);
			ring.erase(replica_key);
			sorted_keys.erase(replica_key);
		}
	}

	int* get_node(const uint32_t& CGgroup_ID) {
		if (ring.empty()) {
			return 0;
		}

		//int hash_value = get_hash(CGgroup_ID);
		uint32_t hash_value = get_hash(CGgroup_ID)%entire_node_num;
		auto it = sorted_keys.lower_bound(hash_value);
		//printf("시작점은 %d 입니다\n", *it);

        for(int i = 0; i < CGSIZE+PARITY; ++i) {
            if (it == sorted_keys.end()) {
                // Wrap around to the beginning of the ring
                it = sorted_keys.begin();
            }
            assigned_devices[i] = ring[*it];
            it++;
        }

		return assigned_devices;
	}
};