#pragma once
#include <Eigen/Dense>

using Eigen::Matrix;

class NeuralNet
{
	Matrix<float, 16, 6> w1;
	Matrix<float, 16, 1> b1;
	Matrix<float, 4, 16> w2;
	Matrix<float, 4, 1> b2;

public:
	NeuralNet();

	Matrix<float, 4, 1> forward(const Matrix<float, 6, 1>& inputs);
	void mutate(float rate = 0.05f, float strength = 0.1f);

private:

};
