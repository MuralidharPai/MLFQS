#include <bits/stdc++.h>
#define EMPTY make_tuple (-1, -1, -1, -1) 
#define QUANTUM 4
#define lli long long
using namespace std;

/* Process information structure */
typedef tuple<lli, lli, lli, lli > process_info_t;
typedef process_info_t process_res_t;
/* Current process execution status */
enum STATUS {FIRST_BURST, CONTINUE, FINISH};
/* Identify process origin i.e which queue or dummy */
enum SELECTOR {MAX_PRIORITY, RR_QUEUE};
/* Comparator function to order processes according to arr_time */ 
bool my_comparator (process_info_t a, process_info_t b) {
	if (get<1>(a) == get<1>(b))
		return (get<3>(a) == get<3>(b) ? get<0>(a) < get<0>(b) : get<3>(a) < get<3>(b));
	return get<1>(a) < get<1>(b);
}
/* Comparator function to order priority queue processes
 * according to (priority, process id) */ 
bool q_comparator (process_info_t a, process_info_t b) {
	return (get<3>(a) == get<3>(b) ? get<0>(a) > get<0>(b) : get<3>(a) > get<3>(b));
}
	
int main (int argc, char **argv) {
	ifstream fin(argv[1]);
	ofstream fout(argv[2]);
	lli num_proc, pid, arr_time, burst_time, priority;
	fin >> num_proc;
	deque<process_info_t > process;
	/* Populate process deque */
	for (lli i = 0; i < num_proc; i++) {
		fin >> pid >> arr_time >> burst_time >> priority;
		process_info_t temp = make_tuple (pid, arr_time, burst_time, priority);
		process.push_back (temp);
	}
	/* Sort deque according to comparator defined above */
	sort (process.begin (), process.end (), my_comparator);
	/* Map to store STATUS, response time, finish time, waiting time
	 * corresponding to process id 
	 */
	map<lli, process_res_t> mp;
	/* Maximum priority queue */
	priority_queue<process_info_t, vector<process_info_t >,
												decltype(&q_comparator) > max_priority(q_comparator);
	/* Round robin queue */
	deque<process_info_t > rr_queue;
	/* Global current time and current quantum time */
	lli curr_time = 0, curr_quantum = 0;
	while (true) {
		/* There are still processes yet to arrive */
		if (! process.empty ()) {
			process_info_t p = process.front (); //Next process in line
			/* Arrival time matches current time */
			if (get<1>(p) == curr_time) {
				/* Insert newly arriving process to the priority queue
				 * and possibly put currently executing process at the back
				 * of the rr_queue if its priority is lower than the new process
				 */
				if (! max_priority.empty () && q_comparator (max_priority.top (), p)
									&& curr_quantum != 0) {
					rr_queue.push_back (max_priority.top ());
					max_priority.pop ();
					/* Reset current time quantum */
					curr_quantum = 0;	
				}
				else if (max_priority.empty ()) {
					if (! rr_queue.empty () && curr_quantum != 0) {
						rr_queue.push_back (rr_queue.front ());
						rr_queue.pop_front ();
					}
					curr_quantum = 0;
				}
				max_priority.push (p);
				/* Make an entry in the map corresponding to the new process */
				mp[get<0>(p)] = make_tuple (FIRST_BURST, curr_time, curr_time, 0);
				process.pop_front ();
			}
			/* This loop is to push all remaining processes arriving at
			 * the same time to the back of the rr_queue
			 */
			while (! process.empty () && get<1>(process.front ()) == curr_time) {
				p = process.front ();
				/* Make an entry in the map corresponding to the new process */
				mp[get<0>(p)] = make_tuple (FIRST_BURST, curr_time, curr_time, 0);
				max_priority.push (p);
				process.pop_front ();
			}
		}
		/* If max_priority queue and rr_queue and process deque are empty then
		 * all processes have finished execution
		 */
		if (max_priority.empty () && rr_queue.empty () && process.empty ())
			break;
		else if (max_priority.empty () && rr_queue.empty ()) {
			curr_time = get<1>(process.front ()); //optimization to avoid wasteful loops
			curr_quantum = 0;
			continue;
		}
		
		int slct;  // Identifies queue to which current process belongs
		/* Select process that is going to run in current time quantum */
		process_info_t p = (! max_priority.empty () ? (slct = MAX_PRIORITY, 
							max_priority.top ()) : (slct = RR_QUEUE, rr_queue.front ()));
		/* Update information of process */	
		if (get<0>(mp[get<0>(p)]) == FIRST_BURST) {
			/* Calculations for response time */
			get<0>(mp[get<0>(p)]) = CONTINUE;
			get<1>(mp[get<0>(p)]) = curr_time - get<1>(mp[get<0>(p)]);
		}
		/* Reduce burst time of current process by 1 */
		(get<2>(p) >= 0 ? get<2>(p)-- : false);
		/* Update the waiting times of all processes whose 
		 * status is not FINISH except for the current executing process
		 */
		for (auto it = mp.begin(); it != mp.end(); it++) {
			if (get<2>(p)!=-1 && (it->first != get<0>(p)) && (get<0>(it->second) != FINISH)) 
				get<3>(it->second)++;
		}
		/* If burst time of current process becomes 0 then update 
		 * finishing time and make the current process a dummy 
		 */
		if (get<2>(p) < 0) {
			get<2>(mp[get<0>(p)]) = curr_time;
			get<0>(mp[get<0>(p)]) = FINISH;
			p = EMPTY;
			curr_quantum = QUANTUM - 1;
			curr_time--;
		} 
		if (get<2>(p) == 0) {
			get<2>(mp[get<0>(p)]) = curr_time + 1;
			get<0>(mp[get<0>(p)]) = FINISH;
			p = EMPTY;
			curr_quantum = QUANTUM - 1;
		}
		/* Push dummy element to indicate finished process */
		if (slct == MAX_PRIORITY) {
			max_priority.pop ();
			max_priority.push (p);
		}
		else if (slct == RR_QUEUE) {
			rr_queue.pop_front ();
			rr_queue.push_front (p);
		}
		/* Update global current time and current quantum time */
		curr_time++; curr_quantum++;
		/* Check if current quantum time is equal to quantum size */	
		if (curr_quantum == QUANTUM) {
			/* If max_priority queue process is switching out */
			if (! max_priority.empty () && (max_priority.top () == EMPTY)) 
				max_priority.pop ();
			else if (! max_priority.empty ()) {
				rr_queue.push_back (max_priority.top ());
				max_priority.pop ();
			}
			/* max_priority queue is empty but rr_queue is not */
			else if (! rr_queue.empty () && (rr_queue.front () == EMPTY))
				rr_queue.pop_front ();
			else if (! rr_queue.empty ()) {
				rr_queue.push_back (rr_queue.front ());
				rr_queue.pop_front ();
			}		
			curr_quantum = 0;
		}
	}
	/* Print process information result */
	for (auto it = mp.begin(); it != mp.end(); it++) {
		fout << it->first << " " << get<1>(it->second) << " ";
		fout << get<2>(it->second) << " " << get<3>(it->second) << endl;
	}
	return 0;	
}
