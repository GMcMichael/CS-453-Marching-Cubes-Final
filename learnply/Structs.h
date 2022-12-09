#pragma once

#ifndef STRUCTS_H
#define STRUCTS_H

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
using namespace std;

namespace myStructs {
	#define EPSILON 1.0e-5

	struct Vector3 {
		float x;
		float y;
		float z;

		Vector3(float _x, float _y, float _z) {
			x = _x;
			y = _y;
			z = _z;
		}

		Vector3(float coord) {
			x = coord;
			y = coord;
			z = coord;
		}

		Vector3() {}

		float Distance(Vector3 other) {
			return Distance(other.x, other.y, other.z);
		}

		float Distance(float otherX, float otherY, float otherZ) {
			return sqrtf(powf(otherX - x, 2) + powf(otherY - y, 2) + powf(otherZ - z, 2));
		}

		std::string Vector3::toString(int precision = 2) {
			stringstream stream;
			//stream.precision(precision);
			//stream << fixed;
			stream << "(" << x << "," << y << "," << z << ")";
			return stream.str();
		}

		bool Equals(const Vector3& other) {
			return Distance(other) <= EPSILON;
		}
	};

	struct Vector4 : Vector3 {
		float w;

		Vector4(float _x, float _y, float _z, float _w) {
			x = _x;
			y = _y;
			z = _z;
			w = _w;
		}

		Vector4(float _x, float _y, float _z) {
			x = _x;
			y = _y;
			z = _z;
			w = 0;
		}

		Vector4(float coord) {
			x = coord;
			y = coord;
			z = coord;
			w = 0;
		}

		Vector4() {}

		std::string Vector4::toString(int precision = 2) {
			stringstream stream;
			//stream.precision(precision);
			//stream << fixed;
			stream << "(" << x << "," << y << "," << z << "): " << w;
			return stream.str();
		}

		bool Equals(const Vector4& other) {
			return Vector3::Equals(other) && (abs(this->w - other.w) <= EPSILON);
		}
	};

	struct Dataset {
	private:
		string name;
		int width;
		int height;
		int depth;
		int channels;
		Vector4* values;

	public:
		string Name() { return name; };
		int Width() { return width; };
		int Height() { return height; };
		int Depth() { return depth; };
		int Channels() { return channels; };
		Vector4* Values() { return values; };

		Dataset(string _name, int _width, int _height, int _depth, int _channels) {
			name = _name;
			width = _width;
			height = _height;
			depth = _depth;
			channels = _channels;
			values = new Vector4[_width * _height * _depth];
		}

		Dataset() {}

		int GetIndex(int x, int y, int z) {
			return x + y * width + z * width * height;
		}

		void AddLayer(unsigned char* layer, int layerIndex, float surface) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					int index = (x + y * width) * channels;

					float r = layer[index];
					float g = layer[index + 1];
					float b = layer[index + 2];

					Vector4 greyscale = Vector4(x/* - (width / 2) */, y/* - (height / 4)*/, layerIndex, ((r + g + b) / 3) - 0);// <= surface ? -1.0f : 1.0f);// ((((r + g + b) / 3) / 255) * 2) - 1);
					//std::cout << "value: " << greyscale.w << std::endl;
					values[GetIndex(x, y, layerIndex)] = greyscale;
				}
			}
		}

		int GetValue(int x, int y, int z) {
			return values ? values[GetIndex(x, y, z)].w : -1;
		}

		void ClearDataset() {
			name = "Default";
			width = -1;
			height = -1;
			depth = -1;
			channels = -1;
			if (values) delete values;
		}
	};

	struct Triangle {
		Vector3 v0;
		Vector3 v1;
		Vector3 v2;

		Triangle(Vector3 _v0, Vector3 _v1, Vector3 _v2) {
			v0 = _v0;
			v1 = _v1;
			v2 = _v2;
		}
	};

	struct Mesh {
		vector<Vector3> vertices;
		vector<unsigned int> triangles;
		vector<float> colors;

		unordered_map<string, unsigned int> uniqueVertices;

		vector<Vector3> analyzedVertices;
		vector<vector<Vector3>> neighboursList;

		std::vector<Vector3> mins;
		std::vector<Vector3> maxs;
		std::vector<Vector3> saddles;

		void AddTriangle(Triangle triangle) {
			AddVertex(triangle.v0, Vector3(1, 0, 0));
			AddVertex(triangle.v1, Vector3(0, 1, 0));
			AddVertex(triangle.v2, Vector3(0, 0, 1));
		}

		void AddVertex(Vector3 v, Vector3 color) {
			vertices.push_back(v);
			triangles.push_back(vertices.size() - 1);
			colors.push_back(color.x);
			colors.push_back(color.y);
			colors.push_back(color.z);

			/*Used to show the mesh with no duplicate vertices, <WARNING> is much slower*/
			//int index = -1;
			//if (uniqueVertices.find(v.toString()) == uniqueVertices.end()) {
			//	//doesnt exist, add it to vertices list
			//	vertices.push_back(v);
			//	colors.push_back(color.x);
			//	colors.push_back(color.y);
			//	colors.push_back(color.z);
			//	index = vertices.size() - 1;
			//	uniqueVertices.insert({ v.toString(), (unsigned int)index });
			//}
			//
			//triangles.push_back(index == -1 ? uniqueVertices[v.toString()] : index);
		}

		void AnalyzeVertices() {
			std::cout << "Making neighbours map from " << vertices.size() << " vertices." << std::endl;
			analyzedVertices.clear();
			neighboursList.clear();
			//for each triangle
			for (int i = 0; i < triangles.size(); i += 3) {
				//for each vertex of the triangle
				for (int j = 0; j < 3; j++) {
					Vector3 v = vertices[triangles[i + j]];
					int index = -1;
					for (int k = 0; k < analyzedVertices.size(); k++) {
						if (v.Equals(analyzedVertices[k])) {
							index = k;
							break;
						}
					}

					if (index == -1) {
						//doesnt exist, add it to list
						if (v.Equals(Vector3(0.5, -0.5, 0.669099))) {
							std::cout << "adding inital vertex: " << v.toString() << "\n" << std::endl;
						}
						vector<Vector3> neighbours;
						neighbours.push_back(vertices[triangles[i + 1]]);
						neighbours.push_back(vertices[triangles[i + 2]]);

						analyzedVertices.push_back(v);
						neighboursList.push_back(neighbours);
					}
					else {
						//does exist, check and add neighbours
						Vector3 v1 = vertices[triangles[i + 1]];
						Vector3 v2 = vertices[triangles[i + 2]];
						bool containsV1 = false;
						bool containsV2 = false;
						for (int k = 0; k < neighboursList[index].size(); k++) {
							if (neighboursList[index][k].Equals(v1)) containsV1 = true;
							if (neighboursList[index][k].Equals(v2)) containsV2 = true;
							if (containsV1 && containsV2) break;
						}
						if(!containsV1) neighboursList[index].push_back(v1);
						if(!containsV2) neighboursList[index].push_back(v2);
						if (v.Equals(Vector3(0.5, -0.5, 0.669099))) {
							std::cout << "vertex already exists, adding neighbours: " << (containsV1 == true ? v1.toString() + ", " : "") << (containsV2 == true ? v2.toString() : "") << "\n" << std::endl;
						}
					}
				}
			}
			//could use neighbour map to generate normals
			std::cout << "Done." << std::endl;
			std::cout << analyzedVertices.size() << " unique vertices.\n" << std::endl;
		}

		void FindCriticalPoints() {
			std::cout << "Finding critical points." << std::endl;
			mins.clear();
			maxs.clear();
			saddles.clear();
			for (int i = 0; i < analyzedVertices.size(); i++) {
				Vector3 currPos = analyzedVertices[i];

				//test if current vertex is a max or min
				bool isMax = true;
				bool isMin = true;
				for (int j = 0; j < neighboursList[i].size(); j++) {
					{
						Vector3 neighbour = neighboursList[i][j];//to find critical points, get all mins that are neighbours and coord should be 1/n(sum(x), sum(y), sum(z))
						if (neighbour.y > currPos.y) isMax = false;
						if (neighbour.y < currPos.y) isMin = false;
			
					}
				}
				//if isMax or isMin add point to list
				if (isMin) {
					mins.push_back(currPos);
					if (currPos.Equals(Vector3(0.5, -0.5, 0.669099))) {
						//std::cout << "found min: " << currPos.toString() << ". neighbours: ";
						//for (int j = 0; j < neighboursList[i].size(); j++) {
						//	std::cout << neighboursList[i][j].toString() << ", ";
						//}
						//std::cout << std::endl;
					}
				}
				if (isMax) {
					maxs.push_back(currPos);
					if (currPos.Equals(Vector3(0.5, -0.5, 0.669099))) {
						std::cout << "found max: " << currPos.toString() << ". neighbours: " << std::endl;
						for (int j = 0; j < neighboursList[i].size(); j++) {
							std::cout << neighboursList[i][j].toString() << ", ";
						}
						std::cout << std::endl;
					}
				}
			}
			std::cout << "Done." << std::endl;
			std::cout << "Found " << mins.size() << " min crit points." << std::endl;
			std::cout << "Found " << maxs.size() << " max crit points.\n" << std::endl;
		}
	};

}

#endif // !STRUCTS_H