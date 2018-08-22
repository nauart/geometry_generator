/*
 * Copyright (c) 2019 Artem Nosach
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>

namespace {

struct Vec {
  float x_;
  float y_;
  float z_;
};

struct Box {
  Vec min_;
  Vec max_;
};

struct Ray {
  Vec pos_;
  Vec dir_;
};

// min < max
float generateValue(const float& min, const float& max) {
  return min + static_cast<float>(std::rand()) /
                   static_cast<float>(RAND_MAX / (max - min));
}

Vec generateVector(const Vec& min, const Vec& max) {
  return {generateValue(min.x_, max.x_), generateValue(min.y_, max.y_),
          generateValue(min.z_, max.z_)};
}

float vectorLength(const Vec& vec) {
  return std::sqrt(vec.x_ * vec.x_ + vec.y_ * vec.y_ + vec.z_ * vec.z_);
}

Vec normalizeVector(const Vec& vec) {
  const float length = vectorLength(vec);
  return {vec.x_ / length, vec.y_ / length, vec.z_ / length};
}

Box generateBox(const Box& min, const Box& max) {
  return {generateVector(min.min_, max.min_),
          generateVector(min.max_, max.max_)};
}

Box getBoxChild(const Box& box, const std::uint8_t& child_index) {
  const Vec half = {(box.max_.x_ - box.min_.x_) / 2.0f,
                    (box.max_.y_ - box.min_.y_) / 2.0f,
                    (box.max_.z_ - box.min_.z_) / 2.0f};

  switch (child_index) {
    case 0u:
      return {{box.min_.x_, box.min_.y_, box.min_.z_},
              {box.max_.x_ - half.x_, box.max_.y_ - half.y_,
               box.max_.z_ - half.z_}};
    case 1u:
      return {{box.min_.x_ + half.x_, box.min_.y_, box.min_.z_},
              {box.max_.x_, box.max_.y_ - half.y_, box.max_.z_ - half.z_}};
    case 2u:
      return {{box.min_.x_, box.min_.y_ + half.y_, box.min_.z_},
              {box.max_.x_ - half.x_, box.max_.y_, box.max_.z_ - half.z_}};
    case 3u:
      return {{box.min_.x_ + half.x_, box.min_.y_ + half.y_, box.min_.z_},
              {box.max_.x_, box.max_.y_, box.max_.z_ - half.z_}};
    case 4u:
      return {{box.min_.x_, box.min_.y_, box.min_.z_ + half.z_},
              {box.max_.x_ - half.x_, box.max_.y_ - half.y_, box.max_.z_}};
    case 5u:
      return {{box.min_.x_ + half.x_, box.min_.y_, box.min_.z_ + half.z_},
              {box.max_.x_, box.max_.y_ - half.y_, box.max_.z_}};
    case 6u:
      return {{box.min_.x_, box.min_.y_ + half.y_, box.min_.z_ + half.z_},
              {box.max_.x_ - half.x_, box.max_.y_, box.max_.z_}};
    case 7u:
      return {
          {box.min_.x_ + half.x_, box.min_.y_ + half.y_, box.min_.z_ + half.z_},
          {box.max_.x_, box.max_.y_, box.max_.z_}};
    default:
      return box;
  }
}

float getBoxDiag(const Box& box) {
  return std::sqrt(((box.max_.x_ - box.min_.x_) * (box.max_.x_ - box.min_.x_)) +
                   ((box.max_.y_ - box.min_.y_) * (box.max_.y_ - box.min_.y_)) +
                   ((box.max_.z_ - box.min_.z_) * (box.max_.z_ - box.min_.z_)));
}

Ray generateRay(const Ray& min, const Ray& max) {
  Vec dir = {0.0f, 0.0f, 0.0f};
  do {
    dir = generateVector(min.dir_, max.dir_);
  } while (vectorLength(dir) == 0.0f);

  return {generateVector(min.pos_, max.pos_), normalizeVector(dir)};
}

Vec getRayPoint(const Ray& ray, const float& dist) {
  return {ray.pos_.x_ + ray.dir_.x_ * dist, ray.pos_.y_ + ray.dir_.y_ * dist,
          ray.pos_.z_ + ray.dir_.z_ * dist};
}

std::uint8_t getIntersectChild(const Box& box, const Vec& point) {
  const Vec box_half = {(box.max_.x_ - box.min_.x_) / 2u,
                        (box.max_.y_ - box.min_.y_) / 2u,
                        (box.max_.z_ - box.min_.z_) / 2u};

  if (point.x_ < box_half.x_) {
    // 0 2 4 6
    if (point.y_ < box_half.y_) {
      // 0 4
      if (point.z_ < box_half.z_) {
        return 0u;
      } else {
        return 4u;
      }
    } else {
      // 2 6
      if (point.z_ < box_half.z_) {
        return 2u;
      } else {
        return 6u;
      }
    }
  } else {
    // 1 3 5 7
    if (point.y_ < box_half.y_) {
      // 1 5
      if (point.z_ < box_half.z_) {
        return 1u;
      } else {
        return 5u;
      }
    } else {
      // 3 7
      if (point.z_ < box_half.z_) {
        return 3u;
      } else {
        return 7u;
      }
    }
  }
}

float getRayBoxIntersection(const Ray& ray, const Box& box,
                            std::uint8_t& intersect_child, Ray& reflect_ray) {
  // http://www.cs.utah.edu/~awilliam/box/box.pdf
  float tmin, tmax, tymin, tymax, tzmin, tzmax;

  const float divx = 1.0f / ray.dir_.x_;
  if (divx >= 0.0f) {
    tmin = (box.min_.x_ - ray.pos_.x_) * divx;
    tmax = (box.max_.x_ - ray.pos_.x_) * divx;
  } else {
    tmin = (box.max_.x_ - ray.pos_.x_) * divx;
    tmax = (box.min_.x_ - ray.pos_.x_) * divx;
  }
  Vec reflect_dir = {-ray.dir_.x_, ray.dir_.y_, ray.dir_.z_};

  const float divy = 1.0f / ray.dir_.y_;
  if (divy >= 0.0f) {
    tymin = (box.min_.y_ - ray.pos_.y_) * divy;
    tymax = (box.max_.y_ - ray.pos_.y_) * divy;
  } else {
    tymin = (box.max_.y_ - ray.pos_.y_) * divy;
    tymax = (box.min_.y_ - ray.pos_.y_) * divy;
  }

  if ((tmin > tymax) || (tymin > tmax)) {
    return 0.0f;
  }
  if (tymin > tmin) {
    tmin = tymin;
    reflect_dir = {ray.dir_.x_, -ray.dir_.y_, ray.dir_.z_};
  }
  if (tymax < tmax) {
    tmax = tymax;
  }

  const float divz = 1.0f / ray.dir_.z_;
  if (divz >= 0.0f) {
    tzmin = (box.min_.z_ - ray.pos_.z_) * divz;
    tzmax = (box.max_.z_ - ray.pos_.z_) * divz;
  } else {
    tzmin = (box.max_.z_ - ray.pos_.z_) * divz;
    tzmax = (box.min_.z_ - ray.pos_.z_) * divz;
  }

  if ((tmin > tzmax) || (tzmin > tmax)) {
    return 0.0f;
  }
  if (tzmin > tmin) {
    tmin = tzmin;
    reflect_dir = {ray.dir_.x_, ray.dir_.y_, -ray.dir_.z_};
  }

  const Vec& point = getRayPoint(ray, tmin);
  intersect_child = getIntersectChild(box, point);
  reflect_ray = {point, reflect_dir};

  return tmin;
}

void printUint(FILE* file, const std::uint8_t& value) {
  fprintf(file, "%u", value);
}

void printFloat(FILE* file, const float& value) {
  fprintf(file, "%f", value);
}

void printVector(FILE* file, const Vec& value) {
  fprintf(file, "{");

  printFloat(file, value.x_);
  fprintf(file, ", ");
  printFloat(file, value.y_);
  fprintf(file, ", ");
  printFloat(file, value.z_);

  fprintf(file, "}");
}

void printBox(FILE* file, const Box& value) {
  fprintf(file, "{");

  printVector(file, value.min_);
  fprintf(file, ", ");
  printVector(file, value.max_);

  fprintf(file, "}");
}

void printRay(FILE* file, const Ray& value) {
  fprintf(file, "{");

  printVector(file, value.pos_);
  fprintf(file, ", ");
  printVector(file, value.dir_);

  fprintf(file, "}");
}

void generateIntersectionData(FILE* file, const Box& box_min,
                              const Box& box_max, const Ray& ray_min,
                              const Ray& ray_max) {
  fprintf(file, "{\n  ");

  const Box& box = generateBox(box_min, box_max);
  printBox(file, box);
  fprintf(file, ",\n  {\n");

  constexpr std::uint8_t childs_count = 8u;
  for (std::uint8_t i = 0u; i < childs_count; ++i) {
    const Box& child_box = getBoxChild(box, i);

    fprintf(file, "    ");
    printBox(file, child_box);
    fprintf(file, ",\n");
  }
  fprintf(file, "  },\n");

  const float diag = getBoxDiag(box);
  fprintf(file, "  ");
  printFloat(file, diag);
  fprintf(file, ",\n");

  Ray ray, reflect_ray;
  std::uint8_t intersect_child = 0u;
  float dist = 0.0f;

  do {
    ray = generateRay(ray_min, ray_max);
    dist = getRayBoxIntersection(ray, box, intersect_child, reflect_ray);
  } while (dist > 0.0f);

  fprintf(file, "  ");
  printRay(file, ray);
  fprintf(file, ",\n");

  do {
    ray = generateRay(ray_min, ray_max);
    dist = getRayBoxIntersection(ray, box, intersect_child, reflect_ray);
  } while (dist <= 0.0f);

  fprintf(file, "  ");
  printRay(file, ray);
  fprintf(file, ",\n");

  fprintf(file, "  ");
  printRay(file, reflect_ray);
  fprintf(file, ",\n");

  fprintf(file, "  ");
  printUint(file, intersect_child);
  fprintf(file, ",\n");

  fprintf(file, "  ");
  printFloat(file, dist);
  fprintf(file, "\n");

  fprintf(file, "},\n");
}

}  // namespace

int main(int argc, char* argv[]) {
  const std::string filename = argc > 1 ? argv[1] : "intersection_data.txt";
  const std::string iterations_str = argc > 2 ? argv[2] : "10";
  const std::size_t iterations_count = std::stoul(iterations_str);

  FILE* file = fopen(filename.c_str(), "w");
  if (NULL == file) {
    return 0;
  }

  std::srand(std::time(NULL));
  for (std::size_t i = 0u; i < iterations_count; ++i) {
    const Box box_min = {
        {-10.0f * (i + 1u), -10.0f * (i + 1u), -10.0f * (i + 1u)},
        {5.0f * (i + 1u), 5.0f * (i + 1u), 5.0f * (i + 1u)}};
    const Box box_max = {
        {-5.0f * (i + 1u), -5.0f * (i + 1u), -5.0f * (i + 1u)},
        {10.0f * (i + 1u), 10.0f * (i + 1u), 10.0f * (i + 1u)}};

    const Ray ray_min = {
        {-100.0f * (i + 1u), -100.0f * (i + 1u), -100.0f * (i + 1u)},
        {-1.0f, -1.0f, -1.0f}};
    const Ray ray_max = {
        {100.0f * (i + 1u), 100.0f * (i + 1u), 100.0f * (i + 1u)},
        {1.0f, 1.0f, 1.0f}};

    generateIntersectionData(file, box_min, box_max, ray_min, ray_max);
  }

  fclose(file);
  return 1;
}
