#include "mesh.h"

#include "common.h"

render::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Face>& faces): vertices_(vertices), faces_(faces)
{
}
