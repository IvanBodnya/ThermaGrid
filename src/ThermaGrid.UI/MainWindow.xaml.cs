using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace ThermaGrid.UI
{
    public partial class MainWindow : Window
    {
        private IntPtr _solver = IntPtr.Zero;
        private CancellationTokenSource? _cts;
        private const int Resolution = 400;

        // For color mapping
        private double _minTemp = 20.0;
        private double _maxTemp = 300.0;

        public MainWindow()
        {
            InitializeComponent();
            CreateSolver();
            RenderHeatmap();
        }

        private void CreateSolver()
        {
            if (_solver != IntPtr.Zero)
                ThermaGridInterop.tg_destroy_solver(_solver);

            _solver = ThermaGridInterop.tg_create_solver(1.0, 20.0, 0.01, 3);

            // Boundaries: 100°C on all sides
            for (int side = 0; side < 4; side++)
                ThermaGridInterop.tg_set_boundary_condition(_solver, side, 0, 100.0);

            // Heat source: 300°C in bottom-right quadrant
            ThermaGridInterop.tg_set_region_temperature(_solver, 0.5, 0.5, 1.0, 1.0, 300.0);
        }

        private void OnStart(object sender, RoutedEventArgs e)
        {
            if (_cts != null) return;

            _cts = new CancellationTokenSource();
            StartButton.IsEnabled = false;
            StopButton.IsEnabled = true;

            var token = _cts.Token;

            Task.Run(() =>
            {
                double dt = 0.001;
                double threshold = 5.0;

                while (!token.IsCancellationRequested)
                {
                    ThermaGridInterop.tg_step_forward(_solver, dt);
                    ThermaGridInterop.tg_adapt_grid(_solver, threshold);

                    Dispatcher.Invoke(() =>
                    {
                        RenderHeatmap();
                        UpdateInfo();
                    });

                    Thread.Sleep(50); // ~20 FPS
                }
            }, token);
        }

        private void OnStop(object sender, RoutedEventArgs e)
        {
            _cts?.Cancel();
            _cts = null;
            StartButton.IsEnabled = true;
            StopButton.IsEnabled = false;
        }

        private void OnReset(object sender, RoutedEventArgs e)
        {
            OnStop(sender, e);
            CreateSolver();
            RenderHeatmap();
            UpdateInfo();
        }

        private void UpdateInfo()
        {
            double time = ThermaGridInterop.tg_get_time(_solver);
            int cells = ThermaGridInterop.tg_get_node_count(_solver);

            TimeText.Text = $"Time: {time:F3} s";
            CellsText.Text = $"Cells: {cells}";
        }

        private void RenderHeatmap()
        {
            double[] buffer = new double[Resolution * Resolution];
            ThermaGridInterop.tg_get_temperature_field(_solver, buffer, Resolution);

            int[] levels = new int[Resolution * Resolution];
            ThermaGridInterop.tg_get_cell_levels(_solver, levels, Resolution);

            // Find min and max temperatures
            double min = double.MaxValue, max = double.MinValue;
            for (int i = 0; i < buffer.Length; i++)
            {
                if (buffer[i] < min) min = buffer[i];
                if (buffer[i] > max) max = buffer[i];
            }
            _minTemp = min;
            _maxTemp = max;

            // Update info text
            MinMaxText.Text = $"Range: {min:F1} – {max:F1} °C";

            // Create bitmap
            int w = Resolution;
            int h = Resolution;
            var bmp = new WriteableBitmap(w, h, 96, 96, PixelFormats.Bgra32, null);
            byte[] pixels = new byte[w * h * 4];

            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    double t = buffer[y * w + x];
                    var (r, g, b) = TemperatureToColor(t, _minTemp, _maxTemp);

                    int idx = (y * w + x) * 4;
                    pixels[idx + 0] = b;  // B
                    pixels[idx + 1] = g;  // G
                    pixels[idx + 2] = r;  // R
                    pixels[idx + 3] = 255; // A
                }
            }

            DrawGridOverlay(pixels, levels, w, h);

            bmp.WritePixels(new Int32Rect(0, 0, w, h), pixels, w * 4, 0);
            HeatmapImage.Source = bmp;

            // Update color bar with the new temperature range
            RenderColorBar();
        }

        private void RenderColorBar()
        {
            const int stripWidth = 20;
            const int stripHeight = 400;

            // --- Render the gradient strip ---
            var bmp = new WriteableBitmap(stripWidth, stripHeight, 96, 96, PixelFormats.Bgra32, null);
            byte[] pixels = new byte[stripWidth * stripHeight * 4];

            for (int y = 0; y < stripHeight; y++)
            {
                // t goes from 1 (top = hot) to 0 (bottom = cold)
                double t = 1.0 - (double)y / (stripHeight - 1);
                double tempValue = _minTemp + t * (_maxTemp - _minTemp);
                var (r, g, b) = TemperatureToColor(tempValue, _minTemp, _maxTemp);

                for (int x = 0; x < stripWidth; x++)
                {
                    int idx = (y * stripWidth + x) * 4;
                    pixels[idx + 0] = b;
                    pixels[idx + 1] = g;
                    pixels[idx + 2] = r;
                    pixels[idx + 3] = 255;
                }
            }

            bmp.WritePixels(new Int32Rect(0, 0, stripWidth, stripHeight), pixels, stripWidth * 4, 0);
            ColorbarImage.Source = bmp;

            // --- Render the labels ---
            ColorbarLabels.Children.Clear();
            ColorbarLabels.Height = stripHeight;

            const int numLabels = 6;
            for (int i = 0; i <= numLabels; i++)
            {
                double t = (double)i / numLabels;
                double tempValue = _minTemp + t * (_maxTemp - _minTemp);

                // t=0 should be at the bottom, t=1 at the top
                double y = (1.0 - t) * (stripHeight - 20);

                var label = new TextBlock
                {
                    Text = $"{tempValue:F0} °C",
                    Foreground = Brushes.White,
                    FontSize = 11,
                    FontFamily = new FontFamily("Consolas")
                };

                Canvas.SetLeft(label, 5);
                Canvas.SetTop(label, y);
                ColorbarLabels.Children.Add(label);
            }
        }

        private static (byte R, byte G, byte B) TemperatureToColor(double t, double min, double max)
        {
            if (max <= min) return (0, 0, 255);

            double norm = (t - min) / (max - min);
            norm = Math.Clamp(norm, 0.0, 1.0);

            // Simple blue → cyan → green → yellow → red gradient
            byte r, g, b;

            if (norm < 0.25)
            {
                // Blue → Cyan
                double f = norm / 0.25;
                r = 0;
                g = (byte)(f * 255);
                b = 255;
            }
            else if (norm < 0.5)
            {
                // Cyan → Green
                double f = (norm - 0.25) / 0.25;
                r = 0;
                g = 255;
                b = (byte)((1 - f) * 255);
            }
            else if (norm < 0.75)
            {
                // Green → Yellow
                double f = (norm - 0.5) / 0.25;
                r = (byte)(f * 255);
                g = 255;
                b = 0;
            }
            else
            {
                // Yellow → Red
                double f = (norm - 0.75) / 0.25;
                r = 255;
                g = (byte)((1 - f) * 255);
                b = 0;
            }

            return (r, g, b);
        }

        private void DrawGridOverlay(byte[] pixels, int[] levels, int width, int height)
        {
            // Grid line color (light gray, high contrast against both blue and red)
            const byte gridR = 240;
            const byte gridG = 240;
            const byte gridB = 240;

            for (var y = 0; y < height; y++)
            {
                var x = 0;
                for (; x < width; x++)
                {
                    // we need to shift y * width since levels is a 1D array, containing 2D data
                    var idx = y * width + x;
                    var level = levels[idx];

                    if (level < 0) continue;

                    var isRightEdge = (x + 1 < width)
                                      && (levels[idx + 1] != level);
                    var isBottomEdge = (y + 1 < height)
                                       && (levels[idx + width] != level);

                    if (!isRightEdge && !isBottomEdge) continue;

                    // Direction-aligned dash pattern
                    var drawDash = isRightEdge switch
                    {
                        true when isBottomEdge => true,
                        true => (y % 3) < 2,
                        _ => (x % 3) < 2
                    };

                    if (!drawDash) continue;

                    // Blend the grid color with the existing pixel
                    // NB: each pixel is 4 bytes, in BGRA order
                    // so pIdx = idx * 4 is the byte offset of the pixel.
                    var pIdx = idx * 4;
                    pixels[pIdx + 0] = gridB;
                    pixels[pIdx + 1] = gridG;
                    pixels[pIdx + 2] = gridR;
                    // Alpha stays 255 (opaque)
                }
            }
        }

        protected override void OnClosed(EventArgs e)
        {
            // Stop the simulation without needing RoutedEventArgs
            _cts?.Cancel();
            _cts = null;

            if (_solver != IntPtr.Zero)
            {
                ThermaGridInterop.tg_destroy_solver(_solver);
                _solver = IntPtr.Zero;
            }

            base.OnClosed(e);
        }
    }
}