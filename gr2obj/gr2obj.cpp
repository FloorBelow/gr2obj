#include <fstream>
#include <iostream>
#include "granny211.h"

struct EsoVert {
	float pos[3];
	granny_uint8 col[4];
	int16_t uv[10];
};

struct MatData {
	char const* diffusePath;
	char const* normalPath;
	char const* detailPath;
	char const* diffuse2Path;
	char const* specularPath;
};

granny_data_type_definition ImportantMatDataType[] = {
	{ GrannyStringMember, "diffusePath" },
	{ GrannyStringMember, "normalPath" },
	{ GrannyStringMember, "detailPath" },
	{ GrannyStringMember, "diffuse2Path" },
	{ GrannyStringMember, "specularPath" },
	{ GrannyEndMember}
};

struct TextureData {
	granny_int32 fileIndex;
};

granny_data_type_definition ImportantTexDataType[] = {
	{ GrannyInt32Member, "FileIndex" },
	{ GrannyEndMember}
};

void TexList(char* file) {
	granny_file* gr = GrannyReadEntireFile(file);
	granny_file_info* info = GrannyGetFileInfo(gr);
	if (info->MaterialCount < 2) { GrannyFreeFile(gr); return; }

	std::ofstream outFile;
	outFile.open("tex.txt", std::ios_base::app);


	char outBuf[256];
	for (int meshCount = 0; meshCount < info->MeshCount; meshCount++) {
		for (int matCount = 0; matCount < info->MaterialCount; matCount++) {
			granny_material* mat = info->Materials[matCount];
			if (mat->MapCount == 0) continue;
			MatData matData;
			GrannyConvertSingleObject(mat->ExtendedData.Type, mat->ExtendedData.Object, ImportantMatDataType, &matData, nullptr);
			for (int map = 0; map < mat->MapCount; map++) {
				granny_texture* tex = mat->Maps[map].Material->Texture;
				TextureData texData;
				GrannyConvertSingleObject(tex->ExtendedData.Type, tex->ExtendedData.Object, ImportantTexDataType, &texData, nullptr);
				if (texData.fileIndex == 0) continue;
				if (strcmp(mat->Maps[map].Usage, "diffuse") == 0) sprintf(outBuf, "%d\t%s\n", texData.fileIndex, matData.diffusePath);
				else if (strcmp(mat->Maps[map].Usage, "normal") == 0) sprintf(outBuf, "%d\t%s\n", texData.fileIndex, matData.normalPath);
				else if (strcmp(mat->Maps[map].Usage, "specular") == 0) sprintf(outBuf, "%d\t%s\n", texData.fileIndex, matData.specularPath);
				else if (strcmp(mat->Maps[map].Usage, "detail") == 0) sprintf(outBuf, "%d\t%s\n", texData.fileIndex, matData.detailPath);
				else if (strcmp(mat->Maps[map].Usage, "diffuse2") == 0) sprintf(outBuf, "%d\t%s\n", texData.fileIndex, matData.diffuse2Path);
				else {
					printf("UNKNOWN MAP %s", mat->Maps[map].Usage); continue;
				}
				outFile << outBuf;
			}
		}
		outFile.flush();
		outFile.close();
		GrannyFreeFile(gr);
	}
}

int main(int argc, char* argv[]) {
	printf("%d\n", argc);
	char outBuf[128];
	for (int arg = 1; arg < argc; arg++) {
		std::string outFileName = std::string(argv[arg]);
		outFileName = outFileName.substr(outFileName.find_last_of('\\') + 1);
		outFileName = outFileName.replace(outFileName.length() - 3, 3, "obj");
		printf("%s\n", outFileName.c_str());

		std::ofstream outFile;
		outFile.open(outFileName);

		outFileName = outFileName.replace(outFileName.length() - 3, 3, "mtl");

		granny_file* gr = GrannyReadEntireFile(argv[arg]);
		granny_file_info* info = GrannyGetFileInfo(gr);

		int vertOffset = 1;
		for (int meshCount = 0; meshCount < info->MeshCount; meshCount++) {
			granny_mesh* mesh = info->Meshes[meshCount];
			printf("%s\n", mesh->Name);
			outFile << "g ";
			outFile << mesh->Name;
			outFile << "\n";

			std::cout << "Exporting vertices\n";
			granny_vertex_data* verts = mesh->PrimaryVertexData;
			void* vert = malloc(64);

			for (int i = 0; i < verts->VertexCount; i++) {
				GrannyGetSingleVertex(verts, i, verts->VertexType, vert);
				EsoVert* esoVert = (EsoVert*)vert;
				sprintf(outBuf, "v %f %f %f\n", esoVert->pos[0], esoVert->pos[1], esoVert->pos[2]);
				outFile << outBuf;
			}

			for (int i = 0; i < verts->VertexCount; i++) {
				GrannyGetSingleVertex(verts, i, verts->VertexType, vert);
				EsoVert* esoVert = (EsoVert*)vert;
				sprintf(outBuf, "vt %f %f\n", (float)esoVert->uv[6] / 1024, (float)esoVert->uv[7] / -1024, esoVert->uv[8], esoVert->uv[9]);
				outFile << outBuf;
			}

			std::cout << "Exporting indices\n";
			granny_tri_topology* faces = mesh->PrimaryTopology;
			for (int g = 0; g < faces->GroupCount; g++) {
				for (int i = faces->Groups[g].TriFirst * 3 + 2; i < (faces->Groups[g].TriFirst + faces->Groups[g].TriCount) * 3; i += 3) {
					if (faces->IndexCount > 0) sprintf(outBuf, "f %d/%d %d/%d %d/%d\n", faces->Indices[i - 2] + vertOffset, faces->Indices[i - 2] + vertOffset, faces->Indices[i - 1] + vertOffset, faces->Indices[i - 1] + vertOffset, faces->Indices[i] + vertOffset, faces->Indices[i] + vertOffset);
					else sprintf(outBuf, "f %d/%d %d/%d %d/%d\n", faces->Indices16[i - 2] + vertOffset, faces->Indices16[i - 2] + vertOffset, faces->Indices16[i - 1] + vertOffset, faces->Indices16[i - 1] + vertOffset, faces->Indices16[i] + vertOffset, faces->Indices16[i] + vertOffset);
					outFile << outBuf;
				}
			}
			vertOffset += verts->VertexCount;
		}

		GrannyFreeFile(gr);
		outFile.close();
		//delete[] outBuf;
		std::cout << "Done!\n\n";
	}
}

/*
int main(int argc, char* argv[]) {

	TexList(argv[1]);
	return 0;

	std::string outFileName = std::string(argv[1]);
	outFileName = outFileName.substr(outFileName.find_last_of('\\') + 1);
	outFileName = outFileName.replace(outFileName.length() - 3, 3, "obj");
	printf("%s\n", outFileName.c_str());

	std::ofstream outFile;
	outFile.open(outFileName);

	outFileName = outFileName.replace(outFileName.length() - 3, 3, "mtl");
	std::ofstream matFile;
	matFile.open(outFileName);

	granny_file* gr = GrannyReadEntireFile(argv[1]);
	granny_file_info* info = GrannyGetFileInfo(gr);

	char outBuf[128];
	sprintf(outBuf, "mtllib %s\n", outFileName.c_str());

	int vertOffset = 1;
	for (int meshCount = 0; meshCount < info->MeshCount; meshCount++) {
		granny_mesh* mesh = info->Meshes[meshCount];
		printf("%s\n", mesh->Name);

		outFile << "o ";
		outFile << mesh->Name;
		outFile << "\n";

		std::cout << "Exporting vertices\n";
		granny_vertex_data* verts = mesh->PrimaryVertexData;
		void* vert = malloc(64);

		for (int i = 0; i < verts->VertexCount; i++) {
			GrannyGetSingleVertex(verts, i, verts->VertexType, vert);
			EsoVert* esoVert = (EsoVert*)vert;
			sprintf(outBuf, "v %f %f %f\n", esoVert->pos[0], esoVert->pos[1], esoVert->pos[2]);
			outFile << outBuf;
		}

		for (int i = 0; i < verts->VertexCount; i++) {
			GrannyGetSingleVertex(verts, i, verts->VertexType, vert);
			EsoVert* esoVert = (EsoVert*)vert;
			sprintf(outBuf, "vt %f %f\n", (float)esoVert->uv[6] / 1024, (float)esoVert->uv[7] / -1024, esoVert->uv[8], esoVert->uv[9]);
			outFile << outBuf;
		}

		std::cout << "Exporting indices\n";
		granny_tri_topology* faces = mesh->PrimaryTopology;
		for (int g = 0; g < faces->GroupCount; g++) {
			

			granny_material* mat = mesh->MaterialBindings[faces->Groups[g].MaterialIndex].Material;
			for (int map = 0; map < mat->MapCount; map++) {
				if (strcmp(mat->Maps[map].Usage, "diffuse") != 0) continue;
				granny_texture* tex = mat->Maps[map].Material->Texture;

				TextureData texData;
				GrannyConvertSingleObject(tex->ExtendedData.Type, tex->ExtendedData.Object, ImportantTexDataType, &texData, nullptr);
				MatData matData;
				GrannyConvertSingleObject(mat->ExtendedData.Type, mat->ExtendedData.Object, ImportantMatDataType, &matData, nullptr);

				printf("%s %d\n", matData.diffusePath, texData.fileIndex);

				std::string matName = std::string(matData.diffusePath);
				matName = matName.substr(matName.find_last_of('\\') + 1);
				matName = matName.substr(0, matName.length() - 4);

				sprintf(outBuf, "usemtl %s\n", matName.c_str());
				outFile << outBuf;
				sprintf(outBuf, "newmtl %s\nmap_Kd %d.dds\nKs 0.5 0.5 0.5\nNs 225.0\n", matName.c_str(), texData.fileIndex);
				matFile << outBuf;
			}
			
			sprintf(outBuf, "g %d\n", faces->Groups[g].MaterialIndex);
			outFile << outBuf;
			for (int i = faces->Groups[g].TriFirst * 3 + 2; i < (faces->Groups[g].TriFirst + faces->Groups[g].TriCount) * 3; i += 3) {
				if(faces->IndexCount > 0) sprintf(outBuf, "f %d/%d %d/%d %d/%d\n", faces->Indices[i - 2] + vertOffset, faces->Indices[i - 2] + vertOffset, faces->Indices[i - 1] + vertOffset, faces->Indices[i - 1] + vertOffset, faces->Indices[i] + vertOffset, faces->Indices[i] + vertOffset);
				else sprintf(outBuf, "f %d/%d %d/%d %d/%d\n", faces->Indices16[i - 2] + vertOffset, faces->Indices16[i - 2] + vertOffset, faces->Indices16[i - 1] + vertOffset, faces->Indices16[i - 1] + vertOffset, faces->Indices16[i] + vertOffset, faces->Indices16[i] + vertOffset);
				outFile << outBuf;
			}
		}

		vertOffset += verts->VertexCount;
		
	}
	std::cout << "Done!\n";
	GrannyFreeFile(gr);
	outFile.close();
	return 0;
}
*/