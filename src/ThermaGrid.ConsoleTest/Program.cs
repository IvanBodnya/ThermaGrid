using System;
using System.Runtime.InteropServices;

namespace ThermaGrid.ConsoleTest
{
    public static class ThermaGridApi
    {
        // Match the exact DLL filename (no "lib" prefix now)
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

    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("========================================");
            Console.WriteLine("  ThermaGrid - C# Interop Test");
            Console.WriteLine("========================================\n");

            try
            {
                // 1. Create solver
                Console.WriteLine("Creating solver...");
                IntPtr solver = ThermaGridApi.tg_create_solver(1.0, 20.0, 0.01, 3);

                if (solver == IntPtr.Zero)
                {
                    Console.WriteLine("ERROR: tg_create_solver returned null!");
                    return;
                }

                Console.WriteLine($"  Solver created at {solver}");
                Console.WriteLine($"  Initial cells: {ThermaGridApi.tg_get_node_count(solver)}\n");

                // 2. Set boundary conditions (Dirichlet 100 C)
                Console.WriteLine("Setting boundary conditions (100 C)...");
                for (int side = 0; side < 4; side++)
                    ThermaGridApi.tg_set_boundary_condition(solver, side, 0, 100.0);

                // 3. Set heat source in bottom-right corner
                Console.WriteLine("Setting heat source (300 C, bottom-right)...");
                ThermaGridApi.tg_set_region_temperature(solver, 0.5, 0.5, 1.0, 1.0, 300.0);

                // 4. Print initial field
                Console.WriteLine("\nInitial temperature field (10x10):");
                PrintField(solver, 10);

                // 5. Run simulation
                Console.WriteLine("\nRunning 100 time steps...");
                for (int i = 0; i < 100; i++)
                    ThermaGridApi.tg_step_forward(solver, 0.001);

                // 6. Adapt grid
                Console.WriteLine("Adapting grid...");
                ThermaGridApi.tg_adapt_grid(solver, 5.0);

                // 7. Print final field
                Console.WriteLine($"\nAfter 100 steps:");
                Console.WriteLine($"  Time:   {ThermaGridApi.tg_get_time(solver):F3} s");
                Console.WriteLine($"  Cells:  {ThermaGridApi.tg_get_node_count(solver)}");
                Console.WriteLine("\nFinal temperature field (10x10):");
                PrintField(solver, 10);

                // 8. Cleanup
                Console.WriteLine("\nDestroying solver...");
                ThermaGridApi.tg_destroy_solver(solver);

                Console.WriteLine("\n[OK] C# interop test complete!");
            }
            catch (DllNotFoundException ex)
            {
                Console.WriteLine($"\n[DLL NOT FOUND] {ex.Message}");
                Console.WriteLine("  Make sure ThermaGrid_Core.dll is in the output folder.");
                Console.WriteLine("  Check: bin/Debug/net8.0/ or bin/Release/net8.0/");
            }
            catch (EntryPointNotFoundException ex)
            {
                Console.WriteLine($"\n[ENTRY POINT NOT FOUND] {ex.Message}");
                Console.WriteLine("  The DLL doesn't export the function you're calling.");
            }
            catch (BadImageFormatException ex)
            {
                Console.WriteLine($"\n[ARCHITECTURE MISMATCH] {ex.Message}");
                Console.WriteLine("  Make sure both the DLL and the C# app target x64.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"\n[ERROR] {ex.GetType().Name}: {ex.Message}");
                Console.WriteLine($"  Stack: {ex.StackTrace}");
            }
        }

        static void PrintField(IntPtr solver, int resolution)
        {
            double[] buffer = new double[resolution * resolution];
            ThermaGridApi.tg_get_temperature_field(solver, buffer, resolution);

            for (int y = 0; y < resolution; y++)
            {
                for (int x = 0; x < resolution; x++)
                    Console.Write($"{buffer[y * resolution + x],6:F1} ");
                Console.WriteLine();
            }
        }
    }
}