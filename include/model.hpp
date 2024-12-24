#ifndef RT_MODEL_HPP
#define RT_MODEL_HPP

#include "triangle.hpp"
#include "material.hpp"
#include "image.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <optional>
#include <map>
#include <memory>

struct Mesh {
  std::vector<std::shared_ptr<Triangle>> faces{};
};

struct Model {
  std::vector<Mesh> meshes{};
};

// return the directory with a trailing '/'
auto get_directory(const std::string& file_path) -> std::string
{
  auto index{ file_path.find_last_of('/') };
  return (index == std::string::npos) ? "./" : file_path.substr(0, index + 1);
}

using MaterialLib = std::map<std::string, std::shared_ptr<Material>>;
auto import_mtllib(const std::string& mtllib_path) -> std::optional<MaterialLib>
{
  auto file = std::ifstream{ mtllib_path };
  if (!file) {
    std::cerr << "Could not open the file " << mtllib_path << '\n';
    return {};
  }

  auto output = MaterialLib{};

  auto line = std::string{};
  while (std::getline(file, line)) {
    if (line.starts_with('#') || line.length() == 0) continue;

    auto line_stream = std::stringstream{ line };
    auto head = std::string{};
    std::getline(line_stream, head, ' ');

    if (head == "newmtl") {
      auto material_name = std::string{};
      std::getline(line_stream, material_name);
      if (!line_stream) {
        std::cerr << "Could not parse the material name on line: " << line << '\n';
        return {};
      }

      auto found_material = false;
      while (std::getline(file, line)) {
        if (line.starts_with('#') || line.length() == 0) continue;

        line_stream.str(line);
        line_stream.clear();

        std::getline(line_stream, head, ' ');
        if (head == "newmtl") {
          std::cerr << "Could not find the diffuse map for material " << material_name << '\n';
          return {};
        }
        else if (head == "Tr" || head == "d" || head == "Tf") {
          auto transparency = 0.0f;
          line_stream >> transparency;
          if (!line_stream) {
            std::cerr << "Could not parse the transparency value on line: " << line << '\n';
            return {};
          }
          
          if (head != "d") transparency = 1.0f - transparency;
          if (transparency < 0.5f) continue;

          auto material = std::make_shared<Dielectric>(1.5f);
          output.insert(std::pair{ std::move(material_name), material });
          found_material = true;
          break;
        }
        else if (head == "map_Kd") {
          auto texture_name = std::string{};
          std::getline(line_stream, texture_name);
          if (!line_stream) {
            std::cerr << "Could not parse the diffuse map name on line: " << line << '\n';
            return {};
          }
          auto image = load_image(get_directory(mtllib_path) + texture_name);
          if (!image) return {};
          auto texture = std::make_shared<ImageTexture>(std::move(*image));
          auto material = std::make_shared<Lambertian>(texture);
          output.insert(std::pair{ std::move(material_name), material });
          found_material = true;
          break;
        }
      }
      if (!found_material) {
        std::cerr << "Could not find the diffuse map for material " << material_name << '\n';
        return {};
      }
    }
  }
  if (output.empty()) {
    std::cerr << "No materials defined on the mtl file " << mtllib_path << '\n';
    return {};
  }
  return output;
}


auto import_model(const std::string& obj_path, float in_scale = 1.0f) -> std::optional<Model>
{
  auto file = std::ifstream{ obj_path };
  if (!file) {
    std::cerr << "Could not open the file: " << obj_path << '\n';
    return {};
  }

  auto positions = std::vector<glm::vec3>{};
  auto normals = std::vector<glm::vec3>{};
  auto texture_coords = std::vector<glm::vec2>{};
  auto current_material = std::string{};
  auto output = Model{};
  auto material_lib = MaterialLib{};

  auto line = std::string{};
  while (std::getline(file, line)) {
    if (line.starts_with('#') || line.length() == 0) continue;

    auto line_stream = std::stringstream{ line };
    auto head = std::string{};

    std::getline(line_stream, head, ' ');

    if (head == "mtllib") {
      auto material_lib_name = std::string{};
      std::getline(line_stream, material_lib_name);
      if (!line_stream) {
        std::cerr << "Could not parse the material lib name on line: " << line << '\n';
        return {};
      }
      auto result = import_mtllib(get_directory(obj_path) + material_lib_name);
      if (!result) return {};
      material_lib = std::move(*result);
    }
    else if (head == "usemtl") {
      auto material_name = std::string{};
      std::getline(line_stream, material_name);
      if (!line_stream) {
        std::cerr << "Could not parse the material name on line: " << line << '\n';
        return {};
      }

      if (!material_lib.contains(material_name)) {
        std::cerr << "Could not find the material " << material_name << " in the material lib."
                  << " Occurred on the line: " << line << '\n';
        return {};
      }

      output.meshes.push_back(Mesh{});
      current_material = material_name;
    }
    else if (head == "v") {
      auto position = glm::vec3{};
      line_stream >> position.x >> position.y >> position.z;
      if (!line_stream) {
        std::cerr << "Could not parse the geometric vertex on line: " << line << '\n';
        return {};
      }
      positions.push_back(position);
    }
    else if (head == "vn") {
      auto normal = glm::vec3{};
      line_stream >> normal.x >> normal.y >> normal.z;
      if (!line_stream) {
        std::cerr << "Could not parse the vertex normal on line: " << line << '\n';
        return {};
      }
      normals.push_back(normal);
    }
    else if (head == "vt") {
      auto texture_coord = glm::vec2{};
      line_stream >> texture_coord.x >> texture_coord.y;
      if (!line_stream) {
        std::cerr << "Could not parse the texture coordinate on line: " << line << '\n';
        return {};
      }
      texture_coords.push_back(texture_coord);
    }
    else if (head == "f") {
      if (output.meshes.size() == 0) {
        std::cerr << "usemtl must be set before a face element\n";
        return {};
      }

      struct Vertex {
        glm::vec3 position{};
        glm::vec3 normal{};
        glm::vec2 texture_coord{};
      };
      
      auto vertices = std::vector<Vertex>{};

      auto position_index = std::size_t{};
      while (line_stream >> position_index) {
        line_stream.get();

        auto texture_coord_index = std::size_t{};
        line_stream >> texture_coord_index;
        line_stream.get();

        auto normal_index = std::size_t{};
        line_stream >> normal_index;

        if (!line_stream) {
          std::cerr << "Could not parse indices on line: " << line << '\n';
          return {};
        }
        if (position_index > positions.size() 
            || texture_coord_index > texture_coords.size()
            || normal_index > normals.size()) 
        {
          std::cerr << "Invalid indices on line: " << line << '\n';
          return {};
        }

        line_stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

        vertices.push_back(Vertex{
          positions.at(position_index - 1),
          normals.at(normal_index - 1),
          texture_coords.at(texture_coord_index - 1)
        });
      }

      for (auto i = 1U; i < vertices.size() - 1; ++i) {
        auto triangle = std::make_shared<Triangle>(
          vertices.at(0).position,
          vertices.at(i).position,
          vertices.at(i + 1).position,
          vertices.at(0).normal,
          vertices.at(i).normal,
          vertices.at(i + 1).normal,
          vertices.at(0).texture_coord,
          vertices.at(i).texture_coord,
          vertices.at(i + 1).texture_coord,
          material_lib.at(current_material)
        );
        output.meshes.back().faces.push_back(triangle);
      }
    }
  }
  if (output.meshes.size() == 0) {
    std::cerr << "Could not import any model meshes on file " << obj_path << '\n';
    return {};
  }

  //calculate model bounding box
  auto min = glm::vec3{ std::numeric_limits<float>::max() };
  auto max = glm::vec3{ std::numeric_limits<float>::min() };
  for (const auto& mesh : output.meshes) {
    for (const auto& face : mesh.faces) {
      auto bb = face->bounding_box();
      min = { std::min(min.x, bb.axes()[0].min), std::min(min.y, bb.axes()[1].min), std::min(min.z, bb.axes()[2].min) };
      max = { std::max(max.x, bb.axes()[0].max), std::max(max.y, bb.axes()[1].max), std::max(max.z, bb.axes()[2].max) };
    }
  }

  // translate the model to the origin, with the bottom of the bounding box at y = 0
  // then scale the model to fit in a unit cube
  auto center = (min + max) / 2.0f;
  auto scale = in_scale / std::max(std::max(max.x - min.x, max.y - min.y), max.z - min.z);  
  auto transform = glm::scale(glm::mat4{1.0f}, glm::vec3{scale});
  transform = glm::translate(transform, -center + center.y * glm::vec3{0.0f, 1.0f, 0.0f});
  for (auto& mesh : output.meshes) {
    for (auto& face : mesh.faces) {
      auto a = glm::vec3{transform * glm::vec4{face->a(), 1.0f}};
      auto b = glm::vec3{transform * glm::vec4{face->b(), 1.0f}};
      auto c = glm::vec3{transform * glm::vec4{face->c(), 1.0f}};
      face = std::make_shared<Triangle>(a, b, c, face->na(), face->nb(), face->nc(), face->ta(), face->tb(), face->tc(), face->material());
    }
  }

  return output;
}

#endif