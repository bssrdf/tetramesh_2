#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include "Math.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define TETLIBRARY
#include "tetgen.h"

//----------------------- tetgen interop -----------------------------------------------------------------

struct nodemesh
{
	std::vector<float3> nodes;
	std::vector<int32_t> faceid;
};


void tetrahedralize_nodes(std::string inputfile, mesh3* outmesh)
{
	tetgenio in, out;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());
	if (!err.empty()) { std::cerr << err << std::endl; }
	if (!ret) { exit(1); }
	std::cout << "# of shapes    : " << shapes.size() << std::endl;
	std::cout << "# of materials : " << materials.size() << std::endl;

	// put points into tetgenio structure...
	in.numberofpoints = attrib.vertices.size() / 3; 
	outmesh->nodenum = in.numberofpoints;
	in.pointlist = new REAL[in.numberofpoints * 3];
	for (int32_t i = 0; i < in.numberofpoints; i++) 
	{
		outmesh->n_x.push_back(attrib.vertices[3 * i + 0]);
		outmesh->n_y.push_back(attrib.vertices[3 * i + 1]);
		outmesh->n_z.push_back(attrib.vertices[3 * i + 2]);
		outmesh->n_index.push_back(i);
		in.pointlist[i * 3 + 0] = attrib.vertices[3 * i + 0];
		in.pointlist[i * 3 + 1] = attrib.vertices[3 * i + 1];
		in.pointlist[i * 3 + 2] = attrib.vertices[3 * i + 2];
	}
	tetrahedralize("fn-nn", &in, &out);
	// ...done.
	
	for (size_t s = 0; s < shapes.size(); s++)
	{
		outmesh->facenum += shapes[s].mesh.num_face_vertices.size();
		outmesh->n_adj.resize(outmesh->facenum);
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) //loop over faces - 66 
		{
			int fv = shapes[s].mesh.num_face_vertices[f];
			for (size_t v = 0; v < fv; v++)
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				int32_t findex = outmesh->n_index[(3 * idx.vertex_index)];
				int32_t curr = outmesh->n_index[3 * idx.vertex_index];

				bool found = false;
				for (int j = 0; j < outmesh->n_adj.at(curr).size(); j++)
				{
					if (outmesh->n_adj.at(curr).at(j) == 0 && found == false) { outmesh->n_adj.at(curr).push_back(f); found = true; }
				}


			}
			index_offset += fv;
		}

	}
	fprintf(stderr, "Finished");
}
