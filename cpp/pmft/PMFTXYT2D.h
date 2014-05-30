#include <boost/python.hpp>
#include <boost/shared_array.hpp>

#include "LinkCell.h"
#include "num_util.h"
#include "trajectory.h"

#ifndef _PMFTXYT2D_H__
#define _PMFTXYT2D_H__

/*! \file PMFTXYT2D.h
    \brief Routines for computing radial density functions
*/

namespace freud { namespace pmft {

//! Computes the RDF (g(r)) for a given set of points
/*! A given set of reference points is given around which the RDF is computed and averaged in a sea of data points.
    Computing the RDF results in an rdf array listing the value of the RDF at each given r, listed in the r array.

    The values of r to compute the rdf at are controlled by the rmax and dr parameters to the constructor. rmax
    determins the maximum r at which to compute g(r) and dr is the step size for each bin.

    <b>2D:</b><br>
    RDF properly handles 2D boxes. As with everything else in freud, 2D points must be passed in as
    3 component vectors x,y,0. Failing to set 0 in the third component will lead to undefined behavior.
*/
class PMFTXYT2D
    {
    public:
        //! Constructor
        PMFTXYT2D(const trajectory::Box& box, float max_x, float max_y, float max_z, float dx, float dy, float dz);

        //! Destructor
        ~PMFTXYT2D();

        //! Get the simulation box
        const trajectory::Box& getBox() const
            {
            return m_box;
            }

        //! Check if a cell list should be used or not
        bool useCells();

        //! Compute the RDF
        void compute(unsigned int *pcf_array,
                     float3 *ref_points,
                     float *ref_orientations,
                     unsigned int Nref,
                     float3 *points,
                     float *orientations,
                     unsigned int Np);

        //! Python wrapper for compute
        void computePy(boost::python::numeric::array pcf_array,
                       boost::python::numeric::array ref_points,
                       boost::python::numeric::array ref_orientations,
                       boost::python::numeric::array points,
                       boost::python::numeric::array orientations);

        //! Get a reference to the last computed pair correlation function
        // boost::shared_array<unsigned int> getPCF()
        //     {
        //     return m_pcf_array;
        //     }

        //! Get a reference to the x array
        boost::shared_array<float> getX()
            {
            return m_x_array;
            }

        //! Get a reference to the y array
        boost::shared_array<float> getY()
            {
            return m_y_array;
            }

        //! Get a reference to the z array
        boost::shared_array<float> getZ()
            {
            return m_z_array;
            }

        //! Python wrapper for getPCF() (returns a copy)
        // boost::python::numeric::array getPCFPy()
        //     {
        //     unsigned int *arr = m_pcf_array.get();
        //     // return num_util::makeNum(arr, m_nbins);
        //     return num_util::makeNum(arr, m_nbins_x*m_nbins_y*m_nbins_z);
        //     }

        //! Python wrapper for getX() (returns a copy)
        boost::python::numeric::array getXPy()
            {
            float *arr = m_x_array.get();
            return num_util::makeNum(arr, m_nbins_x);
            }

        //! Python wrapper for getY() (returns a copy)
        boost::python::numeric::array getYPy()
            {
            float *arr = m_y_array.get();
            return num_util::makeNum(arr, m_nbins_y);
            }

        //! Python wrapper for getZ() (returns a copy)
        boost::python::numeric::array getZPy()
            {
            float *arr = m_z_array.get();
            return num_util::makeNum(arr, m_nbins_z);
            }
    private:
        trajectory::Box m_box;            //!< Simulation box the particles belong in
        float m_max_x;                     //!< Maximum x at which to compute pcf
        float m_max_y;                     //!< Maximum y at which to compute pcf
        float m_max_z;                     //!< Maximum z at which to compute pcf
        float m_dx;                       //!< Step size for x in the computation
        float m_dy;                       //!< Step size for y in the computation
        float m_dz;                       //!< Step size for z in the computation
        locality::LinkCell* m_lc;          //!< LinkCell to bin particles for the computation
        unsigned int m_nbins_x;             //!< Number of x bins to compute pcf over
        unsigned int m_nbins_y;             //!< Number of y bins to compute pcf over
        unsigned int m_nbins_z;             //!< Number of z bins to compute pcf over

        // boost::shared_array<unsigned int> m_pcf_array;         //!< pcf array computed
        boost::shared_array<float> m_x_array;           //!< array of x values that the pcf is computed at
        boost::shared_array<float> m_y_array;           //!< array of y values that the pcf is computed at
        boost::shared_array<float> m_z_array;           //!< array of z values that the pcf is computed at
    };

/*! \internal
    \brief Exports all classes in this file to python
*/
void export_PMFTXYT2D();

}; }; // end namespace freud::pmft

#endif // _PMFTXYT2D_H__
