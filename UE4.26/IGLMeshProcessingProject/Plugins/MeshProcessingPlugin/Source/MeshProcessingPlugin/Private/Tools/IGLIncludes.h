#pragma once


// UnrealMathUtility.h #defines PI and Eigen doesn't like this
#ifdef PI
#undef PI
#endif

// disable some libigl warnings that UBT will consider errors
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4018 4459)
#endif

// set of libigl includes (you can add others here. If you uncomment the warning(pop) below, you may *need* to add them here)
#include <igl/barycenter.h>
#include <igl/cotmatrix.h>
#include <igl/doublearea.h>
#include <igl/grad.h>
#include <igl/massmatrix.h>
#include <igl/per_vertex_normals.h>

// un-disable the warnings
// note: leaving them disabled is bad behavior but it means the igl headers can be included elsewhere w/o having to push/pop the warnings again

//#if defined(_MSC_VER)
//#pragma warning(pop)
//#endif
