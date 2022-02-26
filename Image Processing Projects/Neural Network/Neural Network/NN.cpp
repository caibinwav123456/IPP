#include "stdafx.h"
#include "NN.h"
#include "math.h"
#include "stdlib.h"
#include "time.h"
#include "tinyxml.h"

VecNN operator*(VecNN &vec, MatNN &mat)
{
	VecNN vecret;
	for(int i=0;i<(int)mat.vecs.size();i++)
	{
		ASSERT(mat.vecs[i].elems.size() == vec.elems.size());
		double a = 0;
		for(int j=0;j<(int)vec.elems.size();j++)
		{
			a+=vec.elems[j]*mat.vecs[i].elems[j];
		}
		vecret.elems.push_back(a);
	}
	return vecret;
}

double Sigmoid(double x)
{
	return 2.0/(exp(2*x)+1)-1.0;
}

NeuralNetwork::NeuralNetwork()
{
	m_transfunc = Sigmoid;
	m_thresh = 0.5;
}

NeuralNetwork::~NeuralNetwork()
{

}

void NeuralNetwork::SetDimension(int* dim, int n)
{
	m_dim.clear();
	for(int i=0;i<n;i++)
	{
		m_dim.push_back(dim[i]);
	}
}

void NeuralNetwork::SetThresh(double thresh)
{
	m_thresh = thresh;
}

void NeuralNetwork::Init(double range)
{
	srand((int)time(NULL));
	m_transfer.clear();
	for(int i=0;i<(int)m_dim.size()-1;i++)
	{
		MatNN layer;
		for(int j=0;j<m_dim[i+1];j++)
		{
			VecNN vec;
			for(int k=0;k<m_dim[i];k++)
			{
				vec.elems.push_back((double)rand()/RAND_MAX);
			}
			if(vec.elems.size()>0)
			{
				double s = vec.elems[0];
				for(int k=0;k<m_dim[i]-1;k++)
				{
					vec.elems[k] = (vec.elems[k+1]-vec.elems[k])*range;
				}
				vec.elems[m_dim[i]-1] = (s-vec.elems[m_dim[i]-1])*range;
			}
			layer.vecs.push_back(vec);
		}
		m_transfer.push_back(layer);
	}
}

BOOL NeuralNetwork::Load(char* name)
{
	TiXmlDocument doc;
	if(!doc.LoadFile(name))
		return FALSE;

	TiXmlElement* root = doc.RootElement();
	TiXmlElement* nodeDim = root->FirstChildElement("dim");
	int nLayer = 0;
	nodeDim->Attribute("n", &nLayer);
	TiXmlElement* nodeDimL = nodeDim->FirstChildElement("dim");

	m_dim.clear();
	for(int i=0;i<nLayer;i++)
	{
		ASSERT(nodeDimL != NULL);
		int n;
		nodeDimL->Attribute("n", &n);
		m_dim.push_back(n);
		nodeDimL = nodeDimL->NextSiblingElement("dim");
	}
	TiXmlElement* nodeLayers = root->FirstChildElement("layers");
	TiXmlElement* nodeTrans = nodeLayers->FirstChildElement("trans");
	m_transfer.clear();
	for(int i=0;i<(int)m_dim.size()-1;i++)
	{
		ASSERT(nodeTrans != NULL);
		MatNN trans;
		TiXmlElement* nodeVec = nodeTrans->FirstChildElement("vec");
		for(int j=0;j<m_dim[i+1];j++)
		{
			ASSERT(nodeVec != NULL);
			VecNN vec;
			TiXmlElement* nodeElem = nodeVec->FirstChildElement("elem");
			for(int k=0;k<m_dim[i];k++)
			{
				ASSERT(nodeElem != NULL);
				double elem;
				nodeElem->Attribute("var", &elem);
				vec.elems.push_back(elem);
				nodeElem = nodeElem->NextSiblingElement("elem");
			}
			trans.vecs.push_back(vec);
			nodeVec = nodeVec->NextSiblingElement("vec");
		}
		m_transfer.push_back(trans);
		nodeTrans = nodeTrans->NextSiblingElement("trans");
	}
	return TRUE;
}

BOOL NeuralNetwork::Store(char* name)
{
	TiXmlDocument doc(name);

	TiXmlElement* root = new TiXmlElement("nn");
	TiXmlElement* nodeDim = new TiXmlElement("dim");
	nodeDim->SetAttribute("n", m_dim.size());
	for(int i=0;i<(int)m_dim.size();i++)
	{
		TiXmlElement* nodeDimL = new TiXmlElement("dim");
		nodeDimL->SetAttribute("n", m_dim[i]);
		nodeDim->LinkEndChild(nodeDimL);
	}
	ASSERT(m_transfer.size() == m_dim.size()-1);
	TiXmlElement* nodeLayers = new TiXmlElement("layers");
	for(int i=0;i<(int)m_transfer.size();i++)
	{
		ASSERT((int)m_transfer[i].vecs.size() == m_dim[i+1]);
		TiXmlElement* nodeTrans = new TiXmlElement("trans");
		for(int j=0;j<(int)m_transfer[i].vecs.size();j++)
		{
			ASSERT(m_transfer[i].vecs[j].elems.size() == m_dim[i]);
			TiXmlElement* nodeVec = new TiXmlElement("vec");
			for(int k=0;k<(int)m_transfer[i].vecs[j].elems.size();k++)
			{
				TiXmlElement* nodeElem = new TiXmlElement("elem");
				nodeElem->SetDoubleAttribute("elem", m_transfer[i].vecs[j].elems[k]);
				nodeVec->LinkEndChild(nodeElem);
			}
			nodeTrans->LinkEndChild(nodeVec);
		}
		nodeLayers->LinkEndChild(nodeTrans);
	}
	root->LinkEndChild(nodeDim);
	root->LinkEndChild(nodeLayers);

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "");
	doc.LinkEndChild(decl);
	doc.LinkEndChild(root);

	return doc.SaveFile(name)?TRUE:FALSE;
}

void NeuralNetwork::Compute()
{
	ASSERT(m_input.elems.size() == m_dim[0]);
	ASSERT(m_transfer.size() == m_dim.size()-1);
	m_layers.clear();
	for(int i=0;i<(int)m_transfer.size();i++)
	{
		VecNN* input = NULL;
		VecNN* output = NULL;
		VecNN layer;

		if(i==0)
		{
			input = &m_input;
		}
		else
		{
			input = &(m_layers[i]);
		}
		if(i == m_transfer.size()-1)
		{
			output = &m_output;
		}
		else
		{
			output = &layer;
		}
		*output = *input*m_transfer[i];
		if(m_transfunc)
		{
			for(int j=0;j<(int)output->elems.size();j++)
			{
				output->elems[j] = m_transfunc(output->elems[j]);
			}
		}
		if(i != m_transfer.size())
		{
			m_layers.push_back(layer);
		}
	}
}