#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "interval.h"
#include "hittable.h"
#include "ray.h"

#include <vector>
#include <memory>

using std::make_shared;
using std::make_unique;
using std::move;
using std::shared_ptr;
using std::unique_ptr;

class hittable_list : public hittable
{
    public:
        std::vector<unique_ptr<hittable>> objects;

        hittable_list() {}
        hittable_list(unique_ptr<hittable> object) { add(move(object)); }

        void clear() { objects.clear(); }

        void add(unique_ptr<hittable> object)
        {
            objects.push_back(move(object));
        }
        
        bool hit(const ray& r, interval ray_t, hit_record& rec) const override
        {
            hit_record temp_rec;
            bool hit_anything = false;
            auto closest_so_far = ray_t.max;

            for (const auto& object : objects)
            {
                if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec))
                {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }

            return hit_anything;
        }
};


#endif