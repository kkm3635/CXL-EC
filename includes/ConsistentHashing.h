#include <bits/stdc++.h>
#include <cstdint>

#define CGSIZE 4
#define PARITY 2

using namespace std;

class ConsistentHashRing {
private:
	map<int, uint32_t> ring;
	set<int> sorted_keys;
	int virtual_node;
    int* assigned_devices;

	unsigned int get_hash(const uint32_t& value) {
		hash<uint32_t> hash_function;
		return hash_function(value);
	}

public:
	ConsistentHashRing(int virtual_node = 1) : virtual_node(virtual_node) {
        assigned_devices = new int[CGSIZE+PARITY];
    }

	// Function to add Node in the ring
	void add_node(const uint32_t& device) {
		for (int i = 0; i < virtual_node; ++i) {
			uint32_t replica_key = get_hash(device);
			ring[replica_key] = device;
			sorted_keys.insert(replica_key);
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

		int hash_value = get_hash(CGgroup_ID);
		auto it = sorted_keys.lower_bound(hash_value);

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