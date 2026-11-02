#include "neural_net.h"
#include <random>
#include <fstream>

NeuralNet::NeuralNet()
{
	w1.setRandom();
	b1.setRandom();
	w2.setRandom();
	b2.setRandom();
}

Matrix<float, 4, 1> NeuralNet::forward(const Matrix<float, 6, 1>& inputs)
{
	Eigen::Matrix<float, 16, 1> hidden = (w1 * inputs + b1).cwiseMax(0.0f);

	Eigen::Matrix<float, 4, 1> output = (w2 * hidden + b2).array().tanh();
	return output;
}

void NeuralNet::mutate(float rate, float strength)
{
	std::mt19937 gen(std::random_device{}());
	std::uniform_real_distribution<float> prob(0., 1.);
	std::normal_distribution<float> noise(0., strength);

	auto mutate_matrix = [&](auto& mat)
	{
		for (int r = 0; r < mat.rows(); ++r) 
		{
		
			for (int c = 0; c < mat.cols(); ++c)
			{
				if (prob(gen) < rate)
				{
					mat(r, c) += noise(gen);
				}
			}
		}
	};

	mutate_matrix(w1);
	mutate_matrix(b1);
	mutate_matrix(w2);
	mutate_matrix(b2);

}


bool NeuralNet::save_binary(const std::string& filename) {
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) return false;

	// Записуємо 4 матриці сирими байтами прямо з пам'яті:
	file.write(reinterpret_cast<const char*>(w1.data()), w1.size() * sizeof(float));
	file.write(reinterpret_cast<const char*>(b1.data()), b1.size() * sizeof(float));
	file.write(reinterpret_cast<const char*>(w2.data()), w2.size() * sizeof(float));
	file.write(reinterpret_cast<const char*>(b2.data()), b2.size() * sizeof(float));

	file.close();
	return true;
}

bool NeuralNet::load_binary(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) return false;

	// Зчитуємо байти назад у матриці:
	file.read(reinterpret_cast<char*>(w1.data()), w1.size() * sizeof(float));
	file.read(reinterpret_cast<char*>(b1.data()), b1.size() * sizeof(float));
	file.read(reinterpret_cast<char*>(w2.data()), w2.size() * sizeof(float));
	file.read(reinterpret_cast<char*>(b2.data()), b2.size() * sizeof(float));

	file.close();
	return true;
}