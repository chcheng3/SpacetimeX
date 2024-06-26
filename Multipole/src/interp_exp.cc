#include <stdio.h>

#include "interpolate.hh"

static
void report_interp_error(int ierr)
{
  if (ierr < 0)
  {
    CCTK_VWarn(1, __LINE__, __FILE__, CCTK_THORNSTRING,
        "CCTK_InterpGridArrays returned error code %d",ierr);
  }
}
  
void Multipole_Interp(CCTK_ARGUMENTS,
                      CCTK_REAL xs[], CCTK_REAL ys[], CCTK_REAL zs[],
                      int real_idx, int imag_idx,
                      CCTK_REAL sphere_real[], CCTK_REAL sphere_imag[])
{
  DECLARE_CCTK_ARGUMENTS;
  DECLARE_CCTK_PARAMETERS;

  // Need parameters for the following:
  // ntheta (dtheta = pi/(ntheta) 
  // nphi (dphi = 2pi/(nphi)
  // r (radius of sphere)
  // NOTE: depending on the interval of integration, denominator above may
  // need to be modified to avoid double counting


  CCTK_INT num_input_arrays  = imag_idx == -1 ? 1 : 2;
  CCTK_INT num_output_arrays = imag_idx == -1 ? 1 : 2;
  const CCTK_INT num_dims = 3;
	
	const CCTK_INT num_points = CCTK_MyProc(cctkGH) == 0 ? (ntheta+1)*(nphi+1) : 0; // Only the 0 processor needs the points

  int ierr = -1;

  const void* interp_coords[num_dims] 
    = { (const void *) xs,
        (const void *) ys,
        (const void *) zs };

  const CCTK_INT input_array_indices[2]
    = { real_idx,
        imag_idx };

  // const CCTK_INT output_array_types[2]
  //   = { CCTK_VARIABLE_REAL,
  //       CCTK_VARIABLE_REAL };
  // Interpolation result types: Not used by CarpetX DriverInterp
  CCTK_INT const output_array_types[1] = {0};
  
  // void * output_arrays[2]
  //   = { (void *) sphere_real,
  //       (void *) sphere_imag };

	// Interpolation result
	CCTK_POINTER output_arrays[2];
	output_arrays[0] = sphere_real;
	output_arrays[1] = sphere_imag;

  const int operator_handle = 0; //not used by CarpetX Interpolator
  // Interpolation parameter table
  CCTK_INT operations[1][num_dims];
  for (int var = 0 ; var < num_points; var++) {
  	operations[0][var] = 0;
  }
  
	int operands[1][num_dims];
  for (int var = 0 ; var < num_points; var++) {
		operands[0][var] = var;
  }

  int param_table_handle = Util_TableCreate(UTIL_TABLE_FLAGS_DEFAULT);
  if (param_table_handle < 0)
    CCTK_VERROR("Can't create parameter table: %d", param_table_handle);
  if ((ierr = Util_TableSetInt(param_table_handle, 1, "order")) < 0)
    CCTK_VERROR("Can't set order in parameter table: %d", ierr);
  if ((ierr = Util_TableSetIntArray(param_table_handle, num_points, (int const*const)operands,
                            "operand_indices")) < 0)
    CCTK_VERROR("Can't set operand_indices array in parameter table: %d", ierr);
  if ((ierr = Util_TableSetIntArray(param_table_handle, num_points, (int const*const)operations,
                            "operation_codes")) < 0)
    CCTK_VERROR("Can't set operation_codes array in parameter table: %d", ierr);

  const int coord_system_handle = 0; // not used by CarpetX Interpolator

  // ierr = CCTK_InterpGridArrays(
  //     cctkGH,
  //     num_dims,
  //     operator_handle,
  //     param_table_handle,
  //     coord_system_handle,
  //     CCTK_MyProc(cctkGH) == 0 ? (ntheta+1)*(nphi+1) : 0, // Only the 0 processor needs the points
  //     CCTK_VARIABLE_REAL,
  //     interp_coords,
  //     num_input_arrays,
  //     input_array_indices,
  //     num_output_arrays,
  //     output_array_types,
  //     output_arrays);

	ierr = DriverInterpolate(
	cctkGH, num_dims, operator_handle, param_table_handle, coord_system_handle,
	num_points, CCTK_VARIABLE_REAL, interp_coords, num_input_arrays, (int const * const)input_array_indices,
	num_output_arrays, output_array_types, output_arrays);
	

  report_interp_error(ierr);

  if (imag_idx == -1)
  {
    for (int i = 0; i < (ntheta+1)*(nphi+1); i++)
    {
      sphere_imag[i] = 0;
    }
  }

  Util_TableDestroy(param_table_handle);
}

// // Debugging routine
// void Multipole_InterpVar(CCTK_ARGUMENTS,
//                          CCTK_REAL x[], CCTK_REAL y[], CCTK_REAL z[], const char *var_name,
//                          CCTK_REAL sphere_var[])
// {
//   DECLARE_CCTK_ARGUMENTS;
//   DECLARE_CCTK_PARAMETERS;

//   // Need parameters for the following:
//   // ntheta (dtheta = pi/(ntheta) 
//   // nphi (dphi = 2pi/(nphi)
//   // r (radius of sphere)
//   // NOTE: depending on the interval of integration, denominator above may
//   // need to be modified to avoid double counting

//   const CCTK_INT num_input_arrays = 1;
//   const CCTK_INT num_output_arrays = 1;
//   const CCTK_INT num_dims = 3;
//   int ierr = -1;

//   const void* interp_coords[num_dims] 
//     = { (const void *) x,
//         (const void *) y,
//         (const void *) z };

//   const CCTK_INT input_array_indices[num_input_arrays]
//     = { CCTK_VarIndex(var_name) };

//   const CCTK_INT output_array_types[num_output_arrays]
//     = { CCTK_VARIABLE_REAL };

//   void * output_arrays[num_output_arrays]
//     = { (void *) sphere_var };

//   const int operator_handle = CCTK_InterpHandle(interpolator_name);

//   int param_table_handle = Util_TableCreate(UTIL_TABLE_FLAGS_DEFAULT);
//   ierr = Util_TableSetFromString(param_table_handle, interpolator_pars);

//   const int coord_system_handle = CCTK_CoordSystemHandle(coord_system);

//   ierr = CCTK_InterpGridArrays(
//       cctkGH,
//       num_dims,
//       operator_handle,
//       param_table_handle,
//       coord_system_handle,
//       (ntheta+1)*(nphi+1),
//       CCTK_VARIABLE_REAL,
//       interp_coords,
//       num_input_arrays,
//       input_array_indices,
//       num_output_arrays,
//       output_array_types,
//       output_arrays);
//   report_interp_error(ierr);
// }
