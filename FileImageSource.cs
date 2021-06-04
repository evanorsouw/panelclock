using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Drawing;
using System.Reactive.Subjects;
using System.IO;

namespace WhiteMagic.PanelClock
{
    public class ImageInfo : IImageInfo
    {
        public Image Image { get; set; }
        public DateTime When { get; set; }
        public string Source{ get; set; }
        public string Path { get; set; }
    }

    class DirectoryInfo
    {
        public string Path;
        public int Count;
    }

    public class FileImageSource : IImageSource, IDisposable
    {
        private string _name;
        private string _path;
        private List<DirectoryInfo> _directories;
        private Subject<DirectoryInfo> _directorySubject;
        private Random _rnd = new Random();
        private ImageInfo _defaultImage;

        public FileImageSource(string name, string path)
        {
            _name = name;
            _path = path;

            _directories = new List<DirectoryInfo>();
            _directorySubject = new Subject<DirectoryInfo>();
            _directorySubject.Subscribe(AddDirectory);
            Task.Run(() => ScanDirectories(_path));
        }

        public string Name => _name;

        private void ScanDirectories(string searchPath)
        {
            try
            {
                var files = GetImageFilesFromDirectory(searchPath);
                _directorySubject.OnNext(new DirectoryInfo { Path = searchPath, Count = files.Count() });

                var paths = Directory.GetDirectories(searchPath, "");
                foreach (var path in paths)
                {
                    ScanDirectories(path);
                }
            }
            catch (Exception ex)
            {
            }
        }

        private IEnumerable<string> GetImageFilesFromDirectory(string path)
        {
            try
            {
                return Directory.GetFiles(path).Where(file => file.EndsWith(".jpg") || file.EndsWith(".png"));
            }
            catch (Exception)
            {
                return new string[0];
            }
        }

        private void AddDirectory(DirectoryInfo dir)
        {
            lock (_directories)
            {
                _directories.Add(dir);
            }
        }

        public IImageInfo GetRandomImage()
        {
            DirectoryInfo dir = null;
            var index = 0;
            lock (_directories)
            {
                var max = _directories.Sum(d => d.Count);
                if (max == 0)
                    return _defaultImage;
                index = _rnd.Next(0, max);
                dir = _directories.FirstOrDefault(d => {
                    var hit = index < d.Count;
                    index -= hit ? 0 : d.Count;
                    return hit;
                });
            }
            var file = GetImageFilesFromDirectory(dir.Path).Skip(index).FirstOrDefault();
            try
            {
                return new ImageInfo
                {
                    Path = file,
                    Image = Bitmap.FromFile(file),
                    Source = Name
                };
            }
            catch (Exception)
            {
                return _defaultImage;
            }
        }

        public void Dispose()
        {
            _directorySubject.Dispose();
        }
    }
}
