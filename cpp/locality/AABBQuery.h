// Copyright (c) 2010-2019 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#ifndef AABBQUERY_H
#define AABBQUERY_H

#include <map>
#include <memory>
#include <vector>

#include "AABBTree.h"
#include "Box.h"
#include "NeighborQuery.h"

/*! \file AABBQuery.h
    \brief Build an AABB tree from points and query it for neighbors.
 * A bounding volume hierarchy (BVH) tree is a binary search tree. It is
 * constructed from axis-aligned bounding boxes (AABBs). The AABB for a node in
 * the tree encloses all child AABBs. A leaf AABB holds multiple particles. The
 * tree is constructed in a balanced way using a heuristic to minimize AABB
 * volume. We build one tree per particle type, and use point AABBs for the
 * particles. The neighbor list is built by traversing down the tree with an
 * AABB that encloses the pairwise cutoff for the particle. Periodic boundaries
 * are treated by translating the query AABB by all possible image vectors,
 * many of which are trivially rejected for not intersecting the root node.
 */

namespace freud { namespace locality {

class AABBQuery : public NeighborQuery
{
public:
    //! Constructs the compute
    AABBQuery();

    //! New-style constructor.
    AABBQuery(const box::Box& box, const vec3<float>* points, unsigned int n_points);

    //! Destructor
    ~AABBQuery();

    //! Perform a query based on a set of query parameters.
    /*! Given a QueryArgs object and a set of points to perform a query
     *  with, this function will dispatch the query to the appropriate
     *  querying function. We override the parent function to support
     *  calling the `query` method with the correct signature.
     *
     *  This function should just be called query, but Cython's function
     *  overloading abilities seem buggy at best, so it's easiest to just
     *  rename the function.
     */
    virtual std::shared_ptr<NeighborQueryPerPointIterator> queryWithArgs(const vec3<float> query_point, unsigned int query_point_idx,
                                                                 QueryArgs args) const;

      //! Given a set of points, find the k elements of this data structure
      //  that are the nearest neighbors for each point. Note that due to the
      //  different signature, this is not directly overriding the original
      //  method in NeighborQuery, so we have to explicitly invalidate calling
      //  with that signature.
      virtual std::shared_ptr<NeighborQueryIterator> query(const vec3<float>* query_points, unsigned int n_query_points, unsigned int num_neighbors, bool exclude_ii = false) const
      {
          throw std::runtime_error("AABBQuery k-nearest-neighbor queries must use the function signature that "
                                   "provides r_max and scale guesses.");
      }

      std::shared_ptr<NeighborQueryIterator> query(const vec3<float>* query_points, unsigned int n_query_points, unsigned int num_neighbors,
                                                   float r_max, float scale, bool exclude_ii = false) const
    {
        QueryArgs qargs;
        qargs.mode = QueryArgs::QueryType::nearest;
        qargs.num_neighbors = num_neighbors;
        qargs.r_max = r_max;
        qargs.scale = scale;
        qargs.exclude_ii = exclude_ii;
        return std::make_shared<NeighborQueryIterator>(this, query_points, n_query_points, qargs);
    }

    AABBTree m_aabb_tree; //!< AABB tree of points

protected:
    //! Validate the combination of specified arguments.
    /*! Add to parent function to account for the various arguments
     *  specifically required for AABBQuery nearest neighbor queries.
     */
    virtual void validateQueryArgs(QueryArgs& args) const
    {
        NeighborQuery::validateQueryArgs(args);
        if (args.mode == QueryArgs::nearest)
        {
            if (args.scale == QueryArgs::DEFAULT_SCALE)
            {
                args.scale = float(1.1);
            }
            if (args.r_max == QueryArgs::DEFAULT_R_MAX)
            {
                // By default, we use 1/10 the smallest box dimension as the guessed query distance.
                vec3<float> L = this->getBox().getL();
                float r_max = std::min(L.x, L.y);
                r_max = this->getBox().is2D() ? r_max : std::min(r_max, L.z);
                args.r_max = float(0.1) * r_max;
            }
        }
    }

private:
    //! Driver for tree configuration
    void setupTree(unsigned int N);

    //! Maps particles by local id to their id within their type trees
    void mapParticlesByType();

    //! Driver to build AABB trees
    void buildTree(const vec3<float>* points, unsigned int N);

    std::vector<AABB> m_aabbs; //!< Flat array of AABBs of all types
    box::Box m_box;            //!< Simulation box where the particles belong
};

//! Parent class of AABB iterators that knows how to traverse general AABB tree structures
class AABBIterator : public NeighborQueryPerPointIterator
{
public:
    //! Constructor
    AABBIterator(const AABBQuery* neighbor_query, const vec3<float> query_point, unsigned int query_point_idx, bool exclude_ii)
        : NeighborQueryPerPointIterator(neighbor_query, query_point, query_point_idx, exclude_ii), m_aabb_query(neighbor_query)
    {}

    //! Empty Destructor
    virtual ~AABBIterator() {}

    //! Computes the image vectors to query for
    void updateImageVectors(float r_max, bool _check_r_max = true);

protected:
    const AABBQuery* m_aabb_query;         //!< Link to the AABBQuery object
    std::vector<vec3<float>> m_image_list; //!< List of translation vectors
    unsigned int m_n_images;               //!< The number of image vectors to check
};

//! Iterator that gets nearest neighbors from AABB tree structures
class AABBQueryIterator : public AABBIterator
{
public:
    //// Explicitly indicate which toNeighborList function is used.
    //using NeighborQueryQueryIterator::toNeighborList;

    //! Constructor
    AABBQueryIterator(const AABBQuery* neighbor_query, const vec3<float> query_point, unsigned int query_point_idx,
                      unsigned int num_neighbors, float r, float scale, bool exclude_ii)
        : AABBIterator(neighbor_query, query_point, query_point_idx, exclude_ii), m_num_neighbors(num_neighbors), m_search_extended(false), m_r(r), m_r_cur(r),
          m_scale(scale), m_all_distances()
    {
        updateImageVectors(0);
    }

    //! Empty Destructor
    virtual ~AABBQueryIterator() {}

    //! Get the next element.
    virtual NeighborBond next();

    ////! Create an equivalent new query iterator on a per-particle basis.
    //virtual std::shared_ptr<NeighborQueryIterator> query(unsigned int idx);

protected:
    unsigned int m_count;                           //!< Number of neighbors returned for the current point.
    unsigned int m_num_neighbors;                               //!< Number of nearest neighbors to find
    std::vector<NeighborBond> m_current_neighbors; //!< The current set of found neighbors.
    float m_search_extended; //!< Flag to see whether we've gone past the safe cutoff distance and have to be
                             //!< worried about finding duplicates.
    float m_r;               //!< Ball cutoff distance. Used as a guess.
    float
        m_r_cur; //!< Current search ball cutoff distance in use for the current particle (expands as needed).
    float m_scale; //!< The amount to scale m_r by when the current ball is too small.
    std::map<unsigned int, float> m_all_distances; //!< Hash map of minimum distances found for a given point,
                                                   //!< used when searching beyond maximum safe AABB distance.
};

//! Iterator that gets neighbors in a ball of size r using AABB tree structures
class AABBQueryBallIterator : public AABBIterator
{
public:
    //! Constructor
    AABBQueryBallIterator(const AABBQuery* neighbor_query, const vec3<float> query_point, unsigned int query_point_idx, float r_max,
                          bool exclude_ii, bool _check_r_max = true)
        : AABBIterator(neighbor_query, query_point, query_point_idx, exclude_ii), m_r_max(r_max), cur_image(0), cur_node_idx(0),
          cur_ref_p(0)
    {
        updateImageVectors(m_r_max, _check_r_max);
    }

    //! Empty Destructor
    virtual ~AABBQueryBallIterator() {}

    //! Get the next element.
    virtual NeighborBond next();

    ////! Create an equivalent new query iterator on a per-particle basis.
    //virtual std::shared_ptr<NeighborQueryIterator> query(unsigned int idx);

protected:
    float m_r_max; //!< Search ball cutoff distance.

private:
    unsigned int cur_image;    //!< The current node in the tree.
    unsigned int cur_node_idx; //!< The current node in the tree.
    unsigned int
        cur_ref_p; //!< The current index into the reference particles in the current node of the tree.
};
}; }; // end namespace freud::locality

#endif // AABBQUERY_H
