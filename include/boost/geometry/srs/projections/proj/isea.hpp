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

// This code was entirely written by Nathan Wagner
// and is in the public domain.

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

#ifndef BOOST_GEOMETRY_PROJECTIONS_ISEA_HPP
#define BOOST_GEOMETRY_PROJECTIONS_ISEA_HPP

#include <sstream>

#include <boost/core/ignore_unused.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct isea {};

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace isea
    {

            static const double E = 52.62263186;
            static const double F = 10.81231696;
            //static const double DEG60 = 1.04719755119659774614;
            //static const double DEG120 = 2.09439510239319549229;
            //static const double DEG72 = 1.25663706143591729537;
            //static const double DEG90 = 1.57079632679489661922;
            //static const double DEG144 = 2.51327412287183459075;
            //static const double DEG36 = 0.62831853071795864768;
            //static const double DEG108 = 1.88495559215387594306;
            //static const double DEG180 = geometry::math::pi<double>();
            /* sqrt(5)/M_PI */
            static const double ISEA_SCALE = 0.8301572857837594396028083;
            /* 26.565051177 degrees */
            static const double V_LAT = 0.46364760899944494524;
            /* 52.62263186 */
            static const double E_RAD = 0.91843818702186776133;
            /* 10.81231696 */
            static const double F_RAD = 0.18871053072122403508;
            /* R tan(g) sin(60) */
            static const double TABLE_G = 0.6615845383;
            /* H = 0.25 R tan g = */
            static const double TABLE_H = 0.1909830056;
            static const double RPRIME = 0.91038328153090290025;
            static const double PRECISION = 0.0000000000005;
            static const double ISEA_STD_LAT = 1.01722196792335072101;
            static const double ISEA_STD_LON = .19634954084936207740;

            template <typename T>
            inline T DEG30() { return T(30) * geometry::math::d2r<T>(); }
            template <typename T>
            inline T DEG60() { return T(60) * geometry::math::d2r<T>(); }
            template <typename T>
            inline T DEG120() { return T(120) * geometry::math::d2r<T>(); }
            template <typename T>
            inline T DEG72() { return T(72) * geometry::math::d2r<T>(); }
            template <typename T>
            inline T DEG90() { return geometry::math::half_pi<T>(); }
            template <typename T>
            inline T DEG144() { return T(144) * geometry::math::d2r<T>(); }
            template <typename T>
            inline T DEG36() { return T(36) * geometry::math::d2r<T>(); }
            template <typename T>
            inline T DEG108() { return T(108) * geometry::math::d2r<T>(); }
            template <typename T>
            inline T DEG180() { return geometry::math::pi<T>(); }

            inline bool DOWNTRI(int tri) { return (((tri - 1) / 5) % 2 == 1); }

            /*
             * Proj 4 provides its own entry points into
             * the code, so none of the library functions
             * need to be global
             */

            struct hex {
                    int iso;
                    int x, y, z;
            };

            /* y *must* be positive down as the xy /iso conversion assumes this */
            inline
            int hex_xy(struct hex *h) {
                if (!h->iso) return 1;
                if (h->x >= 0) {
                    h->y = -h->y - (h->x+1)/2;
                } else {
                    /* need to round toward -inf, not toward zero, so x-1 */
                    h->y = -h->y - h->x/2;
                }
                h->iso = 0;

                return 1;
            }

            inline
            int hex_iso(struct hex *h) {
                if (h->iso) return 1;

                if (h->x >= 0) {
                    h->y = (-h->y - (h->x+1)/2);
                } else {
                    /* need to round toward -inf, not toward zero, so x-1 */
                    h->y = (-h->y - (h->x)/2);
                }

                h->z = -h->x - h->y;
                h->iso = 1;
                return 1;
            }

            template <typename T>
            inline
            int hexbin2(T const& width, T x, T y,
                        int *i, int *j) {
                T z, rx, ry, rz;
                T abs_dx, abs_dy, abs_dz;
                int ix, iy, iz, s;
                struct hex h;

                x = x / cos(DEG30<T>()); /* rotated X coord */
                y = y - x / 2.0; /* adjustment for rotated X */

                /* adjust for actual hexwidth */
                x /= width;
                y /= width;

                z = -x - y;

                rx = floor(x + 0.5);
                ix = (int)rx;
                ry = floor(y + 0.5);
                iy = (int)ry;
                rz = floor(z + 0.5);
                iz = (int)rz;

                s = ix + iy + iz;

                if (s) {
                    abs_dx = fabs(rx - x);
                    abs_dy = fabs(ry - y);
                    abs_dz = fabs(rz - z);

                    if (abs_dx >= abs_dy && abs_dx >= abs_dz) {
                        ix -= s;
                    } else if (abs_dy >= abs_dx && abs_dy >= abs_dz) {
                        iy -= s;
                    } else {
                        iz -= s;
                    }
                }
                h.x = ix;
                h.y = iy;
                h.z = iz;
                h.iso = 1;

                hex_xy(&h);
                *i = h.x;
                *j = h.y;
                    return ix * 100 + iy;
            }

            enum isea_poly { ISEA_NONE, ISEA_ICOSAHEDRON = 20 };
            enum isea_topology { ISEA_HEXAGON=6, ISEA_TRIANGLE=3, ISEA_DIAMOND=4 };
            enum isea_address_form { ISEA_GEO, ISEA_Q2DI, ISEA_SEQNUM, ISEA_INTERLEAVE,
                ISEA_PLANE, ISEA_Q2DD, ISEA_PROJTRI, ISEA_VERTEX2DD, ISEA_HEX
            };

            template <typename T>
            struct isea_dgg {
                int    polyhedron; /* ignored, icosahedron */
                T      o_lat, o_lon, o_az; /* orientation, radians */
                int    pole; /* true if standard snyder */
                int    topology; /* ignored, hexagon */
                int    aperture; /* valid values depend on partitioning method */
                int    resolution;
                T      radius; /* radius of the earth in meters, ignored 1.0 */
                int    output; /* an isea_address_form */
                int    triangle; /* triangle of last transformed point */
                int    quad; /* quad of last transformed point */
                unsigned long serial;
            };

            template <typename T>
            struct isea_pt {
                T x, y;
            };

            template <typename T>
            struct isea_geo {
                T lon, lat;
            };

            template <typename T>
            struct isea_address {
                int    type; /* enum isea_address_form */
                int    number;
                T      x,y; /* or i,j or lon,lat depending on type */
            };

            /* ENDINC */

            enum snyder_polyhedron {
                SNYDER_POLY_HEXAGON, SNYDER_POLY_PENTAGON,
                SNYDER_POLY_TETRAHEDRON, SNYDER_POLY_CUBE,
                SNYDER_POLY_OCTAHEDRON, SNYDER_POLY_DODECAHEDRON,
                SNYDER_POLY_ICOSAHEDRON
            };

            template <typename T>
            struct snyder_constants {
                T          g, G, theta, ea_w, ea_a, ea_b, g_w, g_a, g_b;
            };

            template <typename T>
            inline const snyder_constants<T> * constants()
            {
                /* TODO put these in radians to avoid a later conversion */
                static snyder_constants<T> result[] = {
                    {23.80018260, 62.15458023, 60.0, 3.75, 1.033, 0.968, 5.09, 1.195, 1.0},
                    {20.07675127, 55.69063953, 54.0, 2.65, 1.030, 0.983, 3.59, 1.141, 1.027},
                    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                    {37.37736814, 36.0, 30.0, 17.27, 1.163, 0.860, 13.14, 1.584, 1.0}
                };
                return result;
            }
            
            template <typename T>
            inline const isea_geo<T> * vertex()
            {
                static isea_geo<T> result[] = {
                    {0.0, DEG90<T>()},
                    {DEG180<T>(), V_LAT},
                    {-DEG108<T>(), V_LAT},
                    {-DEG36<T>(), V_LAT},
                    {DEG36<T>(), V_LAT},
                    {DEG108<T>(), V_LAT},
                    {-DEG144<T>(), -V_LAT},
                    {-DEG72<T>(), -V_LAT},
                    {0.0, -V_LAT},
                    {DEG72<T>(), -V_LAT},
                    {DEG144<T>(), -V_LAT},
                    {0.0, -DEG90<T>()}
                };
                return result;
            }

            /* TODO make an isea_pt array of the vertices as well */

            static int      tri_v1[] = {0, 0, 0, 0, 0, 0, 6, 7, 8, 9, 10, 2, 3, 4, 5, 1, 11, 11, 11, 11, 11};

            /* triangle Centers */
            template <typename T>
            inline const isea_geo<T> * icostriangles()
            {
                static isea_geo<T> result[] = {
                    {0.0, 0.0},
                    {-DEG144<T>(), E_RAD},
                    {-DEG72<T>(), E_RAD},
                    {0.0, E_RAD},
                    {DEG72<T>(), E_RAD},
                    {DEG144<T>(), E_RAD},
                    {-DEG144<T>(), F_RAD},
                    {-DEG72<T>(), F_RAD},
                    {0.0, F_RAD},
                    {DEG72<T>(), F_RAD},
                    {DEG144<T>(), F_RAD},
                    {-DEG108<T>(), -F_RAD},
                    {-DEG36<T>(), -F_RAD},
                    {DEG36<T>(), -F_RAD},
                    {DEG108<T>(), -F_RAD},
                    {DEG180<T>(), -F_RAD},
                    {-DEG108<T>(), -E_RAD},
                    {-DEG36<T>(), -E_RAD},
                    {DEG36<T>(), -E_RAD},
                    {DEG108<T>(), -E_RAD},
                    {DEG180<T>(), -E_RAD},
                };
                return result;
            }

            template <typename T>
            inline T az_adjustment(int triangle)
            {
                T          adj;

                isea_geo<T> v;
                isea_geo<T> c;

                v = vertex<T>()[tri_v1[triangle]];
                c = icostriangles<T>()[triangle];

                /* TODO looks like the adjustment is always either 0 or 180 */
                /* at least if you pick your vertex carefully */
                adj = atan2(cos(v.lat) * sin(v.lon - c.lon),
                        cos(c.lat) * sin(v.lat)
                        - sin(c.lat) * cos(v.lat) * cos(v.lon - c.lon));
                return adj;
            }

            template <typename T>
            inline isea_pt<T> isea_triangle_xy(int triangle)
            {
                isea_pt<T>  c;
                T Rprime = 0.91038328153090290025;

                triangle = (triangle - 1) % 20;

                c.x = TABLE_G * ((triangle % 5) - 2) * 2.0;
                if (triangle > 9) {
                    c.x += TABLE_G;
                }
                switch (triangle / 5) {
                case 0:
                    c.y = 5.0 * TABLE_H;
                    break;
                case 1:
                    c.y = TABLE_H;
                    break;
                case 2:
                    c.y = -TABLE_H;
                    break;
                case 3:
                    c.y = -5.0 * TABLE_H;
                    break;
                default:
                    /* should be impossible */
                    BOOST_THROW_EXCEPTION( projection_exception() );
                };
                c.x *= Rprime;
                c.y *= Rprime;

                return c;
            }

            /* snyder eq 14 */
            template <typename T>
            inline T sph_azimuth(T const& f_lon, T const& f_lat, T const& t_lon, T const& t_lat)
            {
                T          az;

                az = atan2(cos(t_lat) * sin(t_lon - f_lon),
                       cos(f_lat) * sin(t_lat)
                       - sin(f_lat) * cos(t_lat) * cos(t_lon - f_lon)
                    );
                return az;
            }

            /* coord needs to be in radians */
            template <typename T>
            inline int isea_snyder_forward(isea_geo<T> * ll, isea_pt<T> * out)
            {
                int             i;

                /*
                 * spherical distance from center of polygon face to any of its
                 * vertexes on the globe
                 */
                T          g;

                /*
                 * spherical angle between radius vector to center and adjacent edge
                 * of spherical polygon on the globe
                 */
                T          G;

                /*
                 * plane angle between radius vector to center and adjacent edge of
                 * plane polygon
                 */
                T          theta;

                /* additional variables from snyder */
                T          q, Rprime, H, Ag, Azprime, Az, dprime, f, rho,
                                x, y;

                /* variables used to store intermediate results */
                T          cot_theta, tan_g, az_offset;

                /* how many multiples of 60 degrees we adjust the azimuth */
                int             Az_adjust_multiples;

                snyder_constants<T> c;

                /*
                 * TODO by locality of reference, start by trying the same triangle
                 * as last time
                 */

                /* TODO put these constants in as radians to begin with */
                c = constants<T>()[SNYDER_POLY_ICOSAHEDRON];
                theta = c.theta * geometry::math::d2r<T>();
                g = c.g * geometry::math::d2r<T>();
                G = c.G * geometry::math::d2r<T>();

                for (i = 1; i <= 20; i++) {
                    T          z;
                    isea_geo<T> center;

                    center = icostriangles<T>()[i];

                    /* step 1 */
                    z = acos(sin(center.lat) * sin(ll->lat)
                         + cos(center.lat) * cos(ll->lat) * cos(ll->lon - center.lon));

                    /* not on this triangle */
                    if (z > g + 0.000005) { /* TODO DBL_EPSILON */
                        continue;
                    }
                    Az = sph_azimuth(center.lon, center.lat, ll->lon, ll->lat);

                    /* step 2 */

                    /* This calculates "some" vertex coordinate */
                    az_offset = az_adjustment<T>(i);

                    Az -= az_offset;

                    /* TODO I don't know why we do this.  It's not in snyder */
                    /* maybe because we should have picked a better vertex */
                    if (Az < 0.0) {
                        Az += geometry::math::two_pi<T>();
                    }
                    /*
                     * adjust Az for the point to fall within the range of 0 to
                     * 2(90 - theta) or 60 degrees for the hexagon, by
                     * and therefore 120 degrees for the triangle
                     * of the icosahedron
                     * subtracting or adding multiples of 60 degrees to Az and
                     * recording the amount of adjustment
                     */

                    Az_adjust_multiples = 0;
                    while (Az < 0.0) {
                        Az += DEG120<T>();
                        Az_adjust_multiples--;
                    }
                    while (Az > DEG120<T>() + DBL_EPSILON) {
                        Az -= DEG120<T>();
                        Az_adjust_multiples++;
                    }

                    /* step 3 */
                    cot_theta = 1.0 / tan(theta);
                    tan_g = tan(g);    /* TODO this is a constant */

                    /* Calculate q from eq 9. */
                    /* TODO cot_theta is cot(30) */
                    q = atan2(tan_g, cos(Az) + sin(Az) * cot_theta);

                    /* not in this triangle */
                    if (z > q + 0.000005) {
                        continue;
                    }
                    /* step 4 */

                    /* Apply equations 5-8 and 10-12 in order */

                    /* eq 5 */
                    /* Rprime = 0.9449322893 * R; */
                    /* R' in the paper is for the truncated */
                    Rprime = 0.91038328153090290025;

                    /* eq 6 */
                    H = acos(sin(Az) * sin(G) * cos(g) - cos(Az) * cos(G));

                    /* eq 7 */
                    /* Ag = (Az + G + H - DEG180) * M_PI * R * R / DEG180; */
                    Ag = Az + G + H - DEG180<T>();

                    /* eq 8 */
                    Azprime = atan2(2.0 * Ag, Rprime * Rprime * tan_g * tan_g - 2.0 * Ag * cot_theta);

                    /* eq 10 */
                    /* cot(theta) = 1.73205080756887729355 */
                    dprime = Rprime * tan_g / (cos(Azprime) + sin(Azprime) * cot_theta);

                    /* eq 11 */
                    f = dprime / (2.0 * Rprime * sin(q / 2.0));

                    /* eq 12 */
                    rho = 2.0 * Rprime * f * sin(z / 2.0);

                    /*
                     * add back the same 60 degree multiple adjustment from step
                     * 2 to Azprime
                     */

                    Azprime += DEG120<T>() * Az_adjust_multiples;

                    /* calculate rectangular coordinates */

                    x = rho * sin(Azprime);
                    y = rho * cos(Azprime);

                    /*
                     * TODO
                     * translate coordinates to the origin for the particular
                     * hexagon on the flattened polyhedral map plot
                     */

                    out->x = x;
                    out->y = y;

                    return i;
                }

                /*
                 * should be impossible, this implies that the coordinate is not on
                 * any triangle
                 */

                //fprintf(stderr, "impossible transform: %f %f is not on any triangle\n",
                //    ll->lon * geometry::math::r2d<double>(), ll->lat * geometry::math::r2d<double>());
                std::stringstream ss;
                ss << "impossible transform: " << ll->lon * geometry::math::r2d<T>()
                   << " " << ll->lat * geometry::math::r2d<T>() << " is not on any triangle.";

                BOOST_THROW_EXCEPTION( projection_exception(ss.str()) );

                /* not reached */
                return 0;        /* supresses a warning */
            }

            /*
             * return the new coordinates of any point in orginal coordinate system.
             * Define a point (newNPold) in orginal coordinate system as the North Pole in
             * new coordinate system, and the great circle connect the original and new
             * North Pole as the lon0 longitude in new coordinate system, given any point
             * in orginal coordinate system, this function return the new coordinates.
             */


            /* formula from Snyder, Map Projections: A working manual, p31 */
            /*
             * old north pole at np in new coordinates
             * could be simplified a bit with fewer intermediates
             *
             * TODO take a result pointer
             */
            template <typename T>
            inline isea_geo<T> snyder_ctran(isea_geo<T> * np, isea_geo<T> * pt)
            {
                isea_geo<T> npt;
                T           alpha, phi, lambda, lambda0, beta, lambdap, phip;
                T           sin_phip;
                T           lp_b;    /* lambda prime minus beta */
                T           cos_p, sin_a;

                phi = pt->lat;
                lambda = pt->lon;
                alpha = np->lat;
                beta = np->lon;
                lambda0 = beta;

                cos_p = cos(phi);
                sin_a = sin(alpha);

                /* mpawm 5-7 */
                sin_phip = sin_a * sin(phi) - cos(alpha) * cos_p * cos(lambda - lambda0);

                /* mpawm 5-8b */

                /* use the two argument form so we end up in the right quadrant */
                lp_b = atan2(cos_p * sin(lambda - lambda0),
                   (sin_a * cos_p * cos(lambda - lambda0) + cos(alpha) * sin(phi)));

                lambdap = lp_b + beta;

                /* normalize longitude */
                /* TODO can we just do a modulus ? */
                lambdap = fmod(lambdap, geometry::math::two_pi<T>());
                while (lambdap > geometry::math::pi<T>())
                    lambdap -= geometry::math::two_pi<T>();
                while (lambdap < -geometry::math::pi<T>())
                    lambdap += geometry::math::two_pi<T>();

                phip = asin(sin_phip);

                npt.lat = phip;
                npt.lon = lambdap;

                return npt;
            }

            template <typename T>
            inline isea_geo<T> isea_ctran(isea_geo<T> * np, isea_geo<T> * pt, T const& lon0)
            {
                isea_geo<T> npt;

                np->lon += geometry::math::pi<T>();
                npt = snyder_ctran(np, pt);
                np->lon -= geometry::math::pi<T>();

                npt.lon -= (geometry::math::pi<T>() - lon0 + np->lon);

                /*
                 * snyder is down tri 3, isea is along side of tri1 from vertex 0 to
                 * vertex 1 these are 180 degrees apart
                 */
                npt.lon += geometry::math::pi<T>();
                /* normalize longitude */
                npt.lon = fmod(npt.lon, geometry::math::two_pi<T>());
                while (npt.lon > geometry::math::pi<T>())
                    npt.lon -= geometry::math::two_pi<T>();
                while (npt.lon < -geometry::math::pi<T>())
                    npt.lon += geometry::math::two_pi<T>();

                return npt;
            }

            /* in radians */

            /* fuller's at 5.2454 west, 2.3009 N, adjacent at 7.46658 deg */

            template <typename T>
            inline int isea_grid_init(isea_dgg<T> * g)
            {
                if (!g)
                    return 0;

                g->polyhedron = 20;
                g->o_lat = ISEA_STD_LAT;
                g->o_lon = ISEA_STD_LON;
                g->o_az = 0.0;
                g->aperture = 4;
                g->resolution = 6;
                g->radius = 1.0;
                g->topology = 6;

                return 1;
            }

            template <typename T>
            inline int isea_orient_isea(isea_dgg<T> * g)
            {
                if (!g)
                    return 0;
                g->o_lat = ISEA_STD_LAT;
                g->o_lon = ISEA_STD_LON;
                g->o_az = 0.0;
                return 1;
            }

            template <typename T>
            inline int isea_orient_pole(isea_dgg<T> * g)
            {
                if (!g)
                    return 0;
                g->o_lat = geometry::math::half_pi<T>();
                g->o_lon = 0.0;
                g->o_az = 0;
                return 1;
            }

            template <typename T>
            inline int isea_transform(isea_dgg<T> * g, isea_geo<T> * in,
                                      isea_pt<T> * out)
            {
                isea_geo<T> i, pole;
                int         tri;

                pole.lat = g->o_lat;
                pole.lon = g->o_lon;

                i = isea_ctran(&pole, in, g->o_az);

                tri = isea_snyder_forward(&i, out);
                out->x *= g->radius;
                out->y *= g->radius;
                g->triangle = tri;

                return tri;
            }


            template <typename T>
            inline void isea_rotate(isea_pt<T> * pt, T const& degrees)
            {
                T          rad;

                T          x, y;

                rad = -degrees * geometry::math::d2r<T>();
                while (rad >= geometry::math::two_pi<T>()) rad -= geometry::math::two_pi<T>();
                while (rad <= -geometry::math::two_pi<T>()) rad += geometry::math::two_pi<T>();

                x = pt->x * cos(rad) + pt->y * sin(rad);
                y = -pt->x * sin(rad) + pt->y * cos(rad);

                pt->x = x;
                pt->y = y;
            }

            template <typename T>
            inline int isea_tri_plane(int tri, isea_pt<T> *pt, T const& radius)
            {
                isea_pt<T> tc; /* center of triangle */

                if (DOWNTRI(tri)) {
                    isea_rotate(pt, 180.0);
                }
                tc = isea_triangle_xy<T>(tri);
                tc.x *= radius;
                tc.y *= radius;
                pt->x += tc.x;
                pt->y += tc.y;

                return tri;
            }

            /* convert projected triangle coords to quad xy coords, return quad number */
            template <typename T>
            inline int isea_ptdd(int tri, isea_pt<T> *pt)
            {
                int             downtri, quad;

                downtri = (((tri - 1) / 5) % 2 == 1);
                boost::ignore_unused(downtri);
                quad = ((tri - 1) % 5) + ((tri - 1) / 10) * 5 + 1;

                isea_rotate(pt, downtri ? 240.0 : 60.0);
                if (downtri) {
                    pt->x += 0.5;
                    /* pt->y += cos(30.0 * M_PI / 180.0); */
                    pt->y += .86602540378443864672;
                }
                return quad;
            }

            template <typename T>
            inline int isea_dddi_ap3odd(isea_dgg<T> *g, int quad, isea_pt<T> *pt, isea_pt<T> *di)
            {
                isea_pt<T> v;
                T          hexwidth;
                T          sidelength;    /* in hexes */
                int        d, i;
                int        maxcoord;
                hex        h;

                /* This is the number of hexes from apex to base of a triangle */
                sidelength = (pow(2.0, g->resolution) + 1.0) / 2.0;

                /* apex to base is cos(30deg) */
                hexwidth = cos(geometry::math::pi<T>() / 6.0) / sidelength;

                /* TODO I think sidelength is always x.5, so
                 * (int)sidelength * 2 + 1 might be just as good
                 */
                maxcoord = (int) (sidelength * 2.0 + 0.5);

                v = *pt;
                hexbin2(hexwidth, v.x, v.y, &h.x, &h.y);
                h.iso = 0;
                hex_iso(&h);

                d = h.x - h.z;
                i = h.x + h.y + h.y;

                /*
                 * you want to test for max coords for the next quad in the same
                 * "row" first to get the case where both are max
                 */
                if (quad <= 5) {
                    if (d == 0 && i == maxcoord) {
                        /* north pole */
                        quad = 0;
                        d = 0;
                        i = 0;
                    } else if (i == maxcoord) {
                        /* upper right in next quad */
                        quad += 1;
                        if (quad == 6)
                            quad = 1;
                        i = maxcoord - d;
                        d = 0;
                    } else if (d == maxcoord) {
                        /* lower right in quad to lower right */
                        quad += 5;
                        d = 0;
                    }
                } else if (quad >= 6) {
                    if (i == 0 && d == maxcoord) {
                        /* south pole */
                        quad = 11;
                        d = 0;
                        i = 0;
                    } else if (d == maxcoord) {
                        /* lower right in next quad */
                        quad += 1;
                        if (quad == 11)
                            quad = 6;
                        d = maxcoord - i;
                        i = 0;
                    } else if (i == maxcoord) {
                        /* upper right in quad to upper right */
                        quad = (quad - 4) % 5;
                        i = 0;
                    }
                }

                di->x = d;
                di->y = i;

                g->quad = quad;
                return quad;
            }

            template <typename T>
            inline int isea_dddi(isea_dgg<T> *g, int quad, isea_pt<T> *pt, isea_pt<T> *di)
            {
                isea_pt<T> v;
                T          hexwidth;
                int        sidelength;    /* in hexes */
                hex        h;

                if (g->aperture == 3 && g->resolution % 2 != 0) {
                    return isea_dddi_ap3odd(g, quad, pt, di);
                }
                /* todo might want to do this as an iterated loop */
                if (g->aperture >0) {
                    sidelength = (int) (pow(T(g->aperture), T(g->resolution / 2.0)) + 0.5);
                } else {
                    sidelength = g->resolution;
                }

                hexwidth = 1.0 / sidelength;

                v = *pt;
                isea_rotate(&v, -30.0);
                hexbin2(hexwidth, v.x, v.y, &h.x, &h.y);
                h.iso = 0;
                hex_iso(&h);

                /* we may actually be on another quad */
                if (quad <= 5) {
                    if (h.x == 0 && h.z == -sidelength) {
                        /* north pole */
                        quad = 0;
                        h.z = 0;
                        h.y = 0;
                        h.x = 0;
                    } else if (h.z == -sidelength) {
                        quad = quad + 1;
                        if (quad == 6)
                            quad = 1;
                        h.y = sidelength - h.x;
                        h.z = h.x - sidelength;
                        h.x = 0;
                    } else if (h.x == sidelength) {
                        quad += 5;
                        h.y = -h.z;
                        h.x = 0;
                    }
                } else if (quad >= 6) {
                    if (h.z == 0 && h.x == sidelength) {
                        /* south pole */
                        quad = 11;
                        h.x = 0;
                        h.y = 0;
                        h.z = 0;
                    } else if (h.x == sidelength) {
                        quad = quad + 1;
                        if (quad == 11)
                            quad = 6;
                        h.x = h.y + sidelength;
                        h.y = 0;
                        h.z = -h.x;
                    } else if (h.y == -sidelength) {
                        quad -= 4;
                        h.y = 0;
                        h.z = -h.x;
                    }
                }
                di->x = h.x;
                di->y = -h.z;

                g->quad = quad;
                return quad;
            }

            template <typename T>
            inline int isea_ptdi(isea_dgg<T> *g, int tri, isea_pt<T> *pt,
                                 isea_pt<T> *di)
            {
                isea_pt<T> v;
                int        quad;

                v = *pt;
                quad = isea_ptdd(tri, &v);
                quad = isea_dddi(g, quad, &v, di);
                return quad;
            }

            /* q2di to seqnum */
            template <typename T>
            inline int isea_disn(isea_dgg<T> *g, int quad, isea_pt<T> *di)
            {
                int             sidelength;
                int             sn, height;
                int             hexes;

                if (quad == 0) {
                    g->serial = 1;
                    return g->serial;
                }
                /* hexes in a quad */
                hexes = (int) (pow(T(g->aperture), T(g->resolution)) + 0.5);
                if (quad == 11) {
                    g->serial = 1 + 10 * hexes + 1;
                    return g->serial;
                }
                if (g->aperture == 3 && g->resolution % 2 == 1) {
                    height = (int) (pow(T(g->aperture), T((g->resolution - 1) / 2.0)));
                    sn = ((int) di->x) * height;
                    sn += ((int) di->y) / height;
                    sn += (quad - 1) * hexes;
                    sn += 2;
                } else {
                    sidelength = (int) (pow(T(g->aperture), T(g->resolution / 2.0)) + 0.5);
                    sn = (int) ((quad - 1) * hexes + sidelength * di->x + di->y + 2);
                }

                g->serial = sn;
                return sn;
            }

            /* TODO just encode the quad in the d or i coordinate
             * quad is 0-11, which can be four bits.
             * d' = d << 4 + q, d = d' >> 4, q = d' & 0xf
             */
            /* convert a q2di to global hex coord */
            template <typename T>
            inline int isea_hex(isea_dgg<T> *g, int tri, isea_pt<T> *pt,
                                isea_pt<T> *hex)
            {
                isea_pt<T> v;
#ifdef BOOST_GEOMETRY_PROJECTIONS_FIXME
                int sidelength;
                int d, i, x, y;
#endif // BOOST_GEOMETRY_PROJECTIONS_FIXME
                int quad;

                quad = isea_ptdi(g, tri, pt, &v);

                hex->x = ((int)v.x << 4) + quad;
                hex->y = v.y;

                return 1;
#ifdef BOOST_GEOMETRY_PROJECTIONS_FIXME
                d = (int)v.x;
                i = (int)v.y;

                /* Aperture 3 odd resolutions */
                if (g->aperture == 3 && g->resolution % 2 != 0) {
                    int offset = (int)(pow(T(3.0), T(g->resolution - 1)) + 0.5);

                    d += offset * ((g->quad-1) % 5);
                    i += offset * ((g->quad-1) % 5);

                    if (quad == 0) {
                        d = 0;
                        i = offset;
                    } else if (quad == 11) {
                        d = 2 * offset;
                        i = 0;
                    } else if (quad > 5) {
                        d += offset;
                    }

                    x = (2*d - i) /3;
                    y = (2*i - d) /3;

                    hex->x = x + offset / 3;
                    hex->y = y + 2 * offset / 3;
                    return 1;
                }

                /* aperture 3 even resolutions and aperture 4 */
                sidelength = (int) (pow(T(g->aperture), T(g->resolution / 2.0)) + 0.5);
                if (g->quad == 0) {
                    hex->x = 0;
                    hex->y = sidelength;
                } else if (g->quad == 11) {
                    hex->x = sidelength * 2;
                    hex->y = 0;
                } else {
                    hex->x = d + sidelength * ((g->quad-1) % 5);
                    if (g->quad > 5) hex->x += sidelength;
                    hex->y = i + sidelength * ((g->quad-1) % 5);
                }

                return 1;
#endif // BOOST_GEOMETRY_PROJECTIONS_FIXME
            }

            template <typename T>
            inline isea_pt<T> isea_forward(isea_dgg<T> *g, isea_geo<T> *in)
            {
                int        tri;
                isea_pt<T> out, coord;

                tri = isea_transform(g, in, &out);

                if (g->output == ISEA_PLANE) {
                    isea_tri_plane(tri, &out, g->radius);
                    return out;
                }

                /* convert to isea standard triangle size */
                out.x = out.x / g->radius * ISEA_SCALE;
                out.y = out.y / g->radius * ISEA_SCALE;
                out.x += 0.5;
                out.y += 2.0 * .14433756729740644112;

                switch (g->output) {
                case ISEA_PROJTRI:
                    /* nothing to do, already in projected triangle */
                    break;
                case ISEA_VERTEX2DD:
                    g->quad = isea_ptdd(tri, &out);
                    break;
                case ISEA_Q2DD:
                    /* Same as above, we just don't print as much */
                    g->quad = isea_ptdd(tri, &out);
                    break;
                case ISEA_Q2DI:
                    g->quad = isea_ptdi(g, tri, &out, &coord);
                    return coord;
                    break;
                case ISEA_SEQNUM:
                    isea_ptdi(g, tri, &out, &coord);
                    /* disn will set g->serial */
                    isea_disn(g, g->quad, &coord);
                    return coord;
                    break;
                case ISEA_HEX:
                    isea_hex(g, tri, &out, &coord);
                    return coord;
                    break;
                }

                return out;
            }
            /*
             * Proj 4 integration code follows
             */

            template <typename T>
            struct par_isea
            {
                isea_dgg<T> dgg;
            };

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_isea_spheroid : public base_t_f<base_isea_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_isea<CalculationType> m_proj_parm;

                inline base_isea_spheroid(const Parameters& par)
                    : base_t_f<base_isea_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    isea_pt<CalculationType> out;
                    isea_geo<CalculationType> in;

                    in.lon = lp_lon;
                    in.lat = lp_lat;

                    isea_dgg<CalculationType> copy = this->m_proj_parm.dgg;
                    out = isea_forward(&copy, &in);

                    xy_x = out.x;
                    xy_y = out.y;
                }

                static inline std::string get_name()
                {
                    return "isea_spheroid";
                }

            };

            // Icosahedral Snyder Equal Area
            template <typename Parameters, typename T>
            inline void setup_isea(Parameters& par, par_isea<T>& proj_parm)
            {
                std::string opt;

                    isea_grid_init(&proj_parm.dgg);

                    proj_parm.dgg.output = ISEA_PLANE;
            /*        proj_parm.dgg.radius = par.a; / * otherwise defaults to 1 */
                /* calling library will scale, I think */

                opt = pj_param(par.params, "sorient").s;
                if (! opt.empty()) {
                    if (opt == std::string("isea")) {
                        isea_orient_isea(&proj_parm.dgg);
                    } else if (opt == std::string("pole")) {
                        isea_orient_pole(&proj_parm.dgg);
                    } else {
                        BOOST_THROW_EXCEPTION( projection_exception(-34) );
                    }
                }

                if (pj_param(par.params, "tazi").i) {
                    proj_parm.dgg.o_az = pj_param(par.params, "razi").f;
                }

                if (pj_param(par.params, "tlon_0").i) {
                    proj_parm.dgg.o_lon = pj_param(par.params, "rlon_0").f;
                }

                if (pj_param(par.params, "tlat_0").i) {
                    proj_parm.dgg.o_lat = pj_param(par.params, "rlat_0").f;
                }

                if (pj_param(par.params, "taperture").i) {
                    proj_parm.dgg.aperture = pj_param(par.params, "iaperture").i;
                }

                if (pj_param(par.params, "tresolution").i) {
                    proj_parm.dgg.resolution = pj_param(par.params, "iresolution").i;
                }

                opt = pj_param(par.params, "smode").s;
                if (! opt.empty()) {
                    if (opt == std::string("plane")) {
                        proj_parm.dgg.output = ISEA_PLANE;
                    } else if (opt == std::string("di")) {
                        proj_parm.dgg.output = ISEA_Q2DI;
                    }
                    else if (opt == std::string("dd")) {
                        proj_parm.dgg.output = ISEA_Q2DD;
                    }
                    else if (opt == std::string("hex")) {
                        proj_parm.dgg.output = ISEA_HEX;
                    }
                    else {
                        /* TODO verify error code.  Possibly eliminate magic */
                        BOOST_THROW_EXCEPTION( projection_exception(-34) );
                    }
                }

                if (pj_param(par.params, "trescale").i) {
                    proj_parm.dgg.radius = ISEA_SCALE;
                }

                if (pj_param(par.params, "tresolution").i) {
                    proj_parm.dgg.resolution = pj_param(par.params, "iresolution").i;
                } else {
                    proj_parm.dgg.resolution = 4;
                }

                if (pj_param(par.params, "taperture").i) {
                    proj_parm.dgg.aperture = pj_param(par.params, "iaperture").i;
                } else {
                    proj_parm.dgg.aperture = 3;
                }
            }

    }} // namespace detail::isea
    #endif // doxygen

    /*!
        \brief Icosahedral Snyder Equal Area projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Spheroid
        \par Projection parameters
         - orient (string)
         - azi: Azimuth (or Gamma) (degrees)
         - lon_0: Central meridian (degrees)
         - lat_0: Latitude of origin (degrees)
         - aperture (integer)
         - resolution (integer)
         - mode (string)
         - rescale
        \par Example
        \image html ex_isea.gif
    */
    template <typename CalculationType, typename Parameters>
    struct isea_spheroid : public detail::isea::base_isea_spheroid<CalculationType, Parameters>
    {
        inline isea_spheroid(const Parameters& par) : detail::isea::base_isea_spheroid<CalculationType, Parameters>(par)
        {
            detail::isea::setup_isea(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::isea, isea_spheroid, isea_spheroid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class isea_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_f<isea_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void isea_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("isea", new isea_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_ISEA_HPP

