using System;
using System.Runtime.InteropServices;

namespace ThermaGrid.UI
{
    public static class ThermaGridInterop
    {
        private const string DllName = "ThermaGrid_Core.dll";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr tg_create_solver(
            double domainSize, double initialTemp, double alpha, int maxLevel);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tg_destroy_solver(IntPtr solver);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tg_step_forward(IntPtr solver, double dt);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tg_adapt_grid(IntPtr solver, double threshold);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tg_set_boundary_condition(
            IntPtr solver, int side, int type, double value);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tg_set_region_temperature(
            IntPtr solver, double x0, double y0, double x1, double y1, double temperature);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void tg_get_temperature_field(
            IntPtr solver, double[] buffer, int resolution);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int tg_get_node_count(IntPtr solver);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern double tg_get_time(IntPtr solver);
    }
}