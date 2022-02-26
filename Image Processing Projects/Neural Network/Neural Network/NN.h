#ifndef _NN_H_
#define _NN_H_

#include "vector"
using namespace std;

struct VecNN
{
	vector<double> elems;
};

struct MatNN
{
	vector<VecNN> vecs;
};

VecNN operator*(VecNN &vec, MatNN &mat);

typedef double (*NNFunc)(double);

double Sigmoid(double);

class NeuralNetwork
{
public:
	NeuralNetwork();
	~NeuralNetwork();

	VecNN m_output;
	VecNN m_input;

	void SetNNFunc(NNFunc func);
	void SetDimension(int* dim, int n);
	void SetThresh(double thresh);
	void Init(double range);
	BOOL Load(char* name);
	BOOL Store(char* name);
	void Compute();

private:
	vector<VecNN> m_layers;
	vector<MatNN> m_transfer;

	vector<int> m_dim;
	double m_thresh;

	NNFunc m_transfunc;
};

#endif