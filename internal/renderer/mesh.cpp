#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "graphics/buffer.hpp"
#include "graphics/device.hpp"

#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "mesh.hpp"

void Mesh::createVertexBuffer()
{
    // vertex buffer

    size_t vertexBufferSize = sizeof(Vertex) * vertices.size();

    std::unique_ptr<Buffer> stagingBuffer =
        std::make_unique<Buffer>(*device, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer->copyDataToMemory(vertices.data());

    vertexBuffer = std::make_unique<Buffer>(*device, vertexBufferSize,
                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // transfer from staging buffer to vertex buffer

    vertexBuffer->transferBufferToBuffer(stagingBuffer->handle);
    stagingBuffer.reset();
}

void Mesh::createIndexBuffer()
{
    // index buffer

    size_t indexBufferSize = sizeof(uint16_t) * indices.size();

    std::unique_ptr<Buffer> stagingBuffer =
        std::make_unique<Buffer>(*device, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer->copyDataToMemory(indices.data());

    indexBuffer = std::make_unique<Buffer>(*device, indexBufferSize,
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    indexBuffer->transferBufferToBuffer(stagingBuffer->handle);
    stagingBuffer.reset();
}

Mesh::Mesh(const std::shared_ptr<Device> device, const std::vector<Vertex> &vertices,
           const std::vector<uint16_t> &indices)
    : device(device), vertices(vertices), indices(indices)
{
    createVertexBuffer();
    createIndexBuffer();
}

void Mesh::setVerticesFromAiScene(const aiScene *pScene)
{
    aiMesh *mesh = pScene->mMeshes[0];
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        const aiVector3D pPos = mesh->mVertices[i];
        aiVector3D pUV = aiVector3D(0.f, 0.f, 0.f);
        if (mesh->HasTextureCoords(0))
            pUV = mesh->mTextureCoords[0][i];

        vertices.emplace_back(Vertex({pPos.x, pPos.y, pPos.z}, {0.f, 0.f, 0.f, 1.f}, {pUV.x, pUV.y}));
    }
}
void Mesh::setIndicesFromAiScene(const aiScene *pScene)
{
    aiMesh *mesh = pScene->mMeshes[0];
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace &Face = mesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        indices.push_back(Face.mIndices[0]);
        indices.push_back(Face.mIndices[1]);
        indices.push_back(Face.mIndices[2]);
    }
}

Mesh::Mesh(const std::shared_ptr<Device> device, const char *modelFilename, const char *textureFilename)
    : device(device)
{
    Assimp::Importer importer;
    const aiScene *pScene =
        importer.ReadFile(modelFilename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                                             aiProcess_JoinIdenticalVertices | aiProcess_ForceGenNormals);
    if (!pScene)
    {
        std::cerr << "Failed to load model : " << modelFilename << std::endl;
        return;
    }

    setVerticesFromAiScene(pScene);
    setIndicesFromAiScene(pScene);

    createVertexBuffer();
    createIndexBuffer();

    int texWidth, texHeight, texChannels;
    stbi_uc *textureData = stbi_load(textureFilename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!textureData)
    {
        std::cerr << "Failed to load texture : " << textureFilename << std::endl;
        return;
    }

    texture = std::make_unique<Texture>(device, texWidth, texHeight, textureData, VK_FORMAT_R8G8B8A8_SRGB,
                                        VK_IMAGE_TILING_OPTIMAL, VK_FILTER_NEAREST);

    stbi_image_free(textureData);
}

Mesh::~Mesh()
{
    indexBuffer.reset();
    vertexBuffer.reset();
}
