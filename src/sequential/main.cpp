#include<bits/stdc++.h>

#define SUBPOPULATION_SIZE 300
#define NUM_SLAVES 10
#define MIN_SUBPOPULATION 4
#define UPDATE_PERIOD 25
#define THRESHOLD 80
#define DECAY_RATE 0.3
#define MUTATION_RATE 0.5
#define MIGRATION_PROB 0.05
#define CROSSOVER_RATE 0.9
#define TERMINATION_THRESHOLD 1e-3

const int DIMENSION = 20;

using namespace std;

int GENERATION;

struct node{
    int node_id;
    vector<float> x;
};

vector<vector<node> > subpopulation;
vector<float> contribution;

void master();
void spawn_subpop(int);
node rand_node(int);
float rand_0_1();
vector<node> get_best();
node get_best_node(vector<node>, bool);
void update_contribution(node);
void merge(vector<node>, node);
void split();
void perform_slave_op(int);
bool termination();
vector<node> mutate(vector<node>);
vector<node> crossover(vector<node>,vector<node> );
vector<node> selection(vector<node>,vector<node>);
vector<float> vector_sum(vector<float>, vector<float>);
vector<float> vector_diff(vector<float>, vector<float>);
vector<float> vector_product(float , vector<float>);
float second_norm(vector<float>);
void replace(vector<node>);
bool check_exists(vector<node>, node);
int find_max_conttribution_subpop();
float opt_func(vector<float>);

bool operator==(node a, node b){
	if(a.node_id != b.node_id){
		return false;
	}
	if(a.x.size() != b.x.size()){
		return false;
	}
	for(int i = 0; i < a.x.size(); i++){
		if(a.x[i] != b.x[i]){
			return false;
		}
	}
	return true;
}

int main() {
	master();
	return 0;
}


void master() {
    GENERATION = 0;

    // spawn sub population
    subpopulation.resize(NUM_SLAVES);
    contribution.resize(NUM_SLAVES, 0.0f);

    for(int i =  0; i < NUM_SLAVES; i++){
        spawn_subpop(i);
    }
	vector<node> best;
	node best_node;
	node best_node_prev;
    while (true){

        // perform the code for each node
        for(int slave = 0; slave < subpopulation.size(); slave++){
            perform_slave_op(slave);
        }

        // remove the best individual from each subpopulation
        best = get_best();
        if (rand_0_1() < MIGRATION_PROB){

            // replace randomly from the left individuals
            replace(best);
        }

        // get best of best
        best_node = get_best_node(best, true);

        if( GENERATION % UPDATE_PERIOD == 0){
            // update con(si) from each subpopulation
            update_contribution(best_node);

            merge(best, best_node);    // checks and merge the subpopulation;
            split();    // check and split the subpopulation
        }

        GENERATION++;

        // termination
        if(GENERATION > 1){
        	if(second_norm(vector_diff(best_node.x, best_node_prev.x)) <= TERMINATION_THRESHOLD);
        	break;
        }

        best_node_prev = best_node;
    }

}

void spawn_subpop(int sp){
    subpopulation[sp].resize(SUBPOPULATION_SIZE);
    // initialise with random x values
    int node_counter = 0;
    for(int i = 0 ; i < SUBPOPULATION_SIZE; i++){
        subpopulation[sp][i] = rand_node(node_counter);
    }
}

float rand_0_1(){
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    return r;
}

node rand_node(int node_id){
	node new_node;
	new_node.node_id = node_id;
	for(int i = 0; i < DIMENSION; i++){
		new_node.x.push_back(rand());
	}
    return new_node;
}

vector<node> get_best(){
    vector<node> best;
    for(int sp = 0; sp<subpopulation.size(); sp++){
        best.push_back(get_best_node(subpopulation[sp], true));
    }
    return best;
}

node get_best_node(vector<node> input, bool maximum = true){
    node best_max = input[0];
    node best_min = input[0];

	float best_value_max = opt_func(input[0].x);
	float best_value_min = opt_func(input[0].x);

	for(int i = 1; i < input.size(); i++){
		float value = opt_func(input[i].x);
		if(value > best_value_max){
			best_value_max =value;
			best_max = input[i];
		}

		if(value < best_value_min){
			best_value_min = value;
			best_min = input[i];
		}
	}
	if(maximum){
		return best_max;
	} else {
		return best_min;
	}
}

void update_contribution(node best_node){
    for(int sp = 0; sp < subpopulation.size(); sp++){	// for each subpopulation
        // update the contribution
        if(check_exists(subpopulation[sp], best_node)){
        	contribution[sp] = contribution[sp] + UPDATE_PERIOD * (1.0f - DECAY_RATE);
        } else {
        	contribution[sp] = contribution[sp] - (UPDATE_PERIOD * DECAY_RATE);
        	if(contribution[sp] < 0){
        		contribution[sp] = 0.0f;
        	}
        }
    }
}

bool check_exists(vector<node> v, node n ){
	for(int i = 0;i<v.size(); i++){
		if(v[i].node_id == n.node_id)	return true;
	}
	return false;
}

void replace(vector<node> best){
	for(int i = 0; i < subpopulation.size(); i++){
		// take the best node of a subpopulation ---- best[i]
		// take the non best node of next subpopulation
		int next_subpop = (i+1)%subpopulation.size();
		int id = rand()%subpopulation[next_subpop].size();

		while(best[next_subpop].node_id != subpopulation[next_subpop][id].node_id){
			id = rand()%subpopulation[next_subpop].size();
		}

		// replace with best
		subpopulation[next_subpop][id] = best[next_subpop];
	}
}

void merge(vector<node> best, node best_node){
	// find the max of all contributions
	int max_cont_subpop = find_max_conttribution_subpop();

	// check for merge condition (Nm, Threshold)
	if(contribution[max_cont_subpop] > THRESHOLD && subpopulation.size() >= MIN_SUBPOPULATION){
		// find the worst of the best individuals
		node worst_of_best = get_best_node(best, false);
		int min_subpop = 0;

		// check the subpop it exists in
		for(int i = 0; i < best.size(); i++){
			if(best[i] == worst_of_best){
				min_subpop = i;
			}
		}

		// merge min_subpop to max_cont_subpop
		for(int i = 0; i < subpopulation[min_subpop].size(); i++){
			int r1 = rand() % subpopulation[max_cont_subpop].size();
			int r2 = rand() % subpopulation[max_cont_subpop].size();

			// update the values
			subpopulation[min_subpop][i].x = vector_sum(best_node.x, vector_product(MUTATION_RATE, vector_diff(subpopulation[max_cont_subpop][r1].x, subpopulation[max_cont_subpop][r2].x)));

			// merge
			subpopulation[max_cont_subpop].push_back(subpopulation[min_subpop][i]);
		}
		subpopulation.erase(subpopulation.begin() + min_subpop);
	}
}

int find_max_conttribution_subpop(){
	int max_cont = contribution[0];
	int sp = 0;
	for(int i = 1; i < contribution.size(); i++){
		if(contribution[i] > max_cont){
			max_cont = contribution[i];
			sp = i;
		}
	}
	return sp;
}

void split(){
    // check if contribution is too loo
    for(int i = 0; i < contribution.size(); i++){
    	if(contribution[i] == 0 && subpopulation[i].size() > SUBPOPULATION_SIZE){
    	    // split into two
    	    vector<node> new_subpop;
    	    for(int j = 0; j < subpopulation[i].size(); j++){
    	    	new_subpop.push_back(rand_node(subpopulation[i][j].node_id));
    	    	subpopulation[i].erase(subpopulation[i].begin() + j);
    	    }
    		return;
    	}
    }
}

void perform_slave_op(int slave){
    // perform DE Step at sub_population[slave]
	// mutation
	vector<node> v = mutate(subpopulation[slave]);
	vector<node> u = crossover(v, subpopulation[slave]);
	subpopulation[slave] = selection(u, subpopulation[slave]);
}

vector<node> selection(vector<node> u, vector<node> x ){
	vector<node> result = x;

	for(int i = 0; i < x.size(); i++){
		if(opt_func(u[i].x) <= opt_func(x[i].x)){
			result[i] = u[i];
		} else {
			result[i] = x[i];
		}
	}
	return result;
}

vector<node> crossover(vector<node> v, vector<node> x ){
	vector<node> result = x;

	for(int i = 0; i < x.size(); i++){
		int j_rand = rand() % DIMENSION;
		node n;
		n.node_id = x[i].node_id;
		for(int j = 0; j < DIMENSION; j++){
			if(rand_0_1() <= CROSSOVER_RATE || j_rand == j){
				n.x.push_back(v[i].x[j]);
			} else {
				n.x.push_back(x[i].x[j]);
			}
		}
		result[i] = n;
	}
	return result;
}

vector<node> mutate(vector<node> input){
	// choose 3 random index
	vector<node> result = input;

	for(int i = 0; i < input.size(); i++){
		int r1, r2, r3;
		r1 = r2 = r3 = i;
		while(r1 == i)	r1 = rand() % input.size();
		while(r2 == i)	r2 = rand() % input.size();
		while(r3 == i)	r3 = rand() % input.size();

		result[i].x = (vector_sum(input[r1].x, vector_product(MUTATION_RATE, vector_diff(input[r2].x, input[r3].x))));

	}
	return result;
}

vector<float> vector_sum(vector<float> a, vector<float> b){
	vector<float> result;
	if(a.size() != b.size()){
		cerr<<"INVALID VECTOR SUMMATION"<<endl;
		exit(1);
	}
	for(int i = 0 ; i < a.size(); i++){
			result.push_back(a[i] + b[i]);	// assumption no overflow
	}
	return result;
}

vector<float> vector_diff(vector<float> a, vector<float> b){
	vector<float> result;
	if(a.size() != b.size()){
		cerr<<"INVALID VECTOR SUMMATION"<<endl;
		exit(1);
	}
	for(int i = 0 ; i < a.size(); i++){
			result.push_back(a[i] - b[i]);	// assumption no overflow
	}
	return result;
}

vector<float> vector_product(float s, vector<float> v ){
	vector<float> result;
	for(int i = 0 ; i < v.size(); i++){
		result.push_back(s * v[i]);
	}
	return result;
}

float second_norm(vector<float> input){
	float norm = 0;
	for(int i = 0; i < input.size() ; i++){
		norm += input[i]*input[i];
	}
	return norm;
}


float opt_func(vector<float> input){
	return 0.0f;
}

bool termination(){
	return false;
}
