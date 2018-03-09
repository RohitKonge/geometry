// Boost.Geometry - gis-projections (based on PROJ4)

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018.
// Modifications copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Last updated version of proj: 5.0.0

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef BOOST_GEOMETRY_PROJECTIONS_CHAMB_HPP
#define BOOST_GEOMETRY_PROJECTIONS_CHAMB_HPP

#include <boost/geometry/util/math.hpp>
#include <cstdio>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>
#include <boost/geometry/srs/projections/impl/aasincos.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct chamb {};

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace chamb
    {

            //static const double THIRD = 0.333333333333333333;
            static const double TOL = 1e-9;

            // specific for 'chamb'
            template <typename T>
            struct VECT { T r, Az; };
            template <typename T>
            struct XY { T x, y; };

            template <typename T>
            struct par_chamb
            {
                struct { /* control point data */
                    T phi, lam;
                    T cosphi, sinphi;
                    VECT<T> v;
                    XY<T>   p;
                    T Az;
                } c[3];
                XY<T> p;
                T beta_0, beta_1, beta_2;
            };

            /* distance and azimuth from point 1 to point 2 */
            template <typename T>
            inline VECT<T> vect(T const& dphi, T const& c1, T const& s1, T const& c2, T const& s2, T const& dlam)
            {
                VECT<T> v;
                T cdl, dp, dl;

                cdl = cos(dlam);
                if (fabs(dphi) > 1. || fabs(dlam) > 1.)
                    v.r = aacos(s1 * s2 + c1 * c2 * cdl);
                else { /* more accurate for smaller distances */
                    dp = sin(.5 * dphi);
                    dl = sin(.5 * dlam);
                    v.r = 2. * aasin(sqrt(dp * dp + c1 * c2 * dl * dl));
                }
                if (fabs(v.r) > TOL)
                    v.Az = atan2(c2 * sin(dlam), c1 * s2 - s1 * c2 * cdl);
                else
                    v.r = v.Az = 0.;
                return v;
            }

            /* law of cosines */
            template <typename T>
            inline T lc(T const& b, T const& c, T const& a)
            {
                return aacos(.5 * (b * b + c * c - a * a) / (b * c));
            }

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_chamb_spheroid : public base_t_f<base_chamb_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_chamb<CalculationType> m_proj_parm;

                inline base_chamb_spheroid(const Parameters& par)
                    : base_t_f<base_chamb_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    static const CalculationType THIRD = detail::THIRD<CalculationType>();

                    CalculationType sinphi, cosphi, a;
                    VECT<CalculationType> v[3];
                    int i, j;

                    sinphi = sin(lp_lat);
                    cosphi = cos(lp_lat);
                    for (i = 0; i < 3; ++i) { /* dist/azimiths from control */
                        v[i] = vect(lp_lat - this->m_proj_parm.c[i].phi, this->m_proj_parm.c[i].cosphi, this->m_proj_parm.c[i].sinphi,
                            cosphi, sinphi, lp_lon - this->m_proj_parm.c[i].lam);
                        if (v[i].r == 0.0)
                            break;
                        v[i].Az = adjlon(v[i].Az - this->m_proj_parm.c[i].v.Az);
                    }
                    if (i < 3) /* current point at control point */
                        { xy_x = this->m_proj_parm.c[i].p.x; xy_y = this->m_proj_parm.c[i].p.y; }
                    else { /* point mean of intersepts */
                        { xy_x = this->m_proj_parm.p.x; xy_y = this->m_proj_parm.p.y; }
                        for (i = 0; i < 3; ++i) {
                            j = i == 2 ? 0 : i + 1;
                            a = lc(this->m_proj_parm.c[i].v.r, v[i].r, v[j].r);
                            if (v[i].Az < 0.)
                                a = -a;
                            if (! i) { /* coord comp unique to each arc */
                                xy_x += v[i].r * cos(a);
                                xy_y -= v[i].r * sin(a);
                            } else if (i == 1) {
                                a = this->m_proj_parm.beta_1 - a;
                                xy_x -= v[i].r * cos(a);
                                xy_y -= v[i].r * sin(a);
                            } else {
                                a = this->m_proj_parm.beta_2 - a;
                                xy_x += v[i].r * cos(a);
                                xy_y += v[i].r * sin(a);
                            }
                        }
                        xy_x *= THIRD; /* mean of arc intercepts */
                        xy_y *= THIRD;
                    }
                }

                static inline std::string get_name()
                {
                    return "chamb_spheroid";
                }

            };

            // Chamberlin Trimetric
            template <typename Parameters, typename T>
            inline void setup_chamb(Parameters& par, par_chamb<T>& proj_parm)
            {
                static const T ONEPI = detail::ONEPI<T>();

                int i, j;
                char line[10];

                for (i = 0; i < 3; ++i) { /* get control point locations */
                    (void)sprintf(line, "rlat_%d", i+1);
                    proj_parm.c[i].phi = pj_param(par.params, line).f;
                    (void)sprintf(line, "rlon_%d", i+1);
                    proj_parm.c[i].lam = pj_param(par.params, line).f;
                    proj_parm.c[i].lam = adjlon(proj_parm.c[i].lam - par.lam0);
                    proj_parm.c[i].cosphi = cos(proj_parm.c[i].phi);
                    proj_parm.c[i].sinphi = sin(proj_parm.c[i].phi);
                }
                for (i = 0; i < 3; ++i) { /* inter ctl pt. distances and azimuths */
                    j = i == 2 ? 0 : i + 1;
                    proj_parm.c[i].v = vect(proj_parm.c[j].phi - proj_parm.c[i].phi, proj_parm.c[i].cosphi, proj_parm.c[i].sinphi,
                        proj_parm.c[j].cosphi, proj_parm.c[j].sinphi, proj_parm.c[j].lam - proj_parm.c[i].lam);
                    if (proj_parm.c[i].v.r == 0.0)
                        BOOST_THROW_EXCEPTION( projection_exception(-25) );
                    /* co-linearity problem ignored for now */
                }
                proj_parm.beta_0 = lc(proj_parm.c[0].v.r, proj_parm.c[2].v.r, proj_parm.c[1].v.r);
                proj_parm.beta_1 = lc(proj_parm.c[0].v.r, proj_parm.c[1].v.r, proj_parm.c[2].v.r);
                proj_parm.beta_2 = ONEPI - proj_parm.beta_0;
                proj_parm.p.y = 2. * (proj_parm.c[0].p.y = proj_parm.c[1].p.y = proj_parm.c[2].v.r * sin(proj_parm.beta_0));
                proj_parm.c[2].p.y = 0.;
                proj_parm.c[0].p.x = - (proj_parm.c[1].p.x = 0.5 * proj_parm.c[0].v.r);
                proj_parm.p.x = proj_parm.c[2].p.x = proj_parm.c[0].p.x + proj_parm.c[2].v.r * cos(proj_parm.beta_0);

                par.es = 0.;
            }

    }} // namespace detail::chamb
    #endif // doxygen

    /*!
        \brief Chamberlin Trimetric projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Miscellaneous
         - Spheroid
         - no inverse
        \par Projection parameters
         - lat_1: Latitude of control point 1 (degrees)
         - lon_1: Longitude of control point 1 (degrees)
         - lat_2: Latitude of control point 2 (degrees)
         - lon_2: Longitude of control point 2 (degrees)
         - lat_3: Latitude of control point 3 (degrees)
         - lon_3: Longitude of control point 3 (degrees)
        \par Example
        \image html ex_chamb.gif
    */
    template <typename CalculationType, typename Parameters>
    struct chamb_spheroid : public detail::chamb::base_chamb_spheroid<CalculationType, Parameters>
    {
        inline chamb_spheroid(const Parameters& par) : detail::chamb::base_chamb_spheroid<CalculationType, Parameters>(par)
        {
            detail::chamb::setup_chamb(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::chamb, chamb_spheroid, chamb_spheroid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class chamb_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_f<chamb_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void chamb_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("chamb", new chamb_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_CHAMB_HPP

