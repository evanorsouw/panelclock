using System;
using System.Threading.Tasks;

namespace WhiteMagic.PanelClock.Extensions
{
    public static class ObservableExtensions
    {
        #region SubscribeOnLatest

        /// <summary>
        /// Subscribe and retain only the last event while busy processing.
        /// </summary>
        public static IDisposable SubscribeOnLatest<T>(this IObservable<T> source, Func<T, Task> onNext)
        {
            return source.Subscribe(CreateOnLatestWrapper(onNext), exception => { }, () => { });
        }

        /// <summary>
        /// Subscribe and retain only the last event while busy processing.
        /// </summary>
        public static IDisposable SubscribeOnLatest<T>(this IObservable<T> source, Action<T> onNext)
        {
            return source.Subscribe(CreateOnLatestWrapper<T>(value =>
            {
                onNext(value);
                return Task.CompletedTask;
            }));
        }

        /// <summary>
        /// Subscribe and retain only the last event while busy processing.
        /// </summary>
        public static IDisposable SubscribeOnLatest<T>(this IObservable<T> source, Func<T, Task> onNext, Action<Exception> onError)
        {
            return source.Subscribe(CreateOnLatestWrapper(onNext), onError, () => { });
        }

        /// <summary>
        /// Subscribe and retain only the last event while busy processing.
        /// </summary>
        public static IDisposable SubscribeOnLatest<T>(this IObservable<T> source, Action<T> onNext, Action<Exception> onError)
        {
            return source.Subscribe(CreateOnLatestWrapper<T>(value =>
            {
                onNext(value);
                return Task.CompletedTask;
            }), onError);
        }

        /// <summary>
        /// Subscribe and retain only the last event while busy processing.
        /// </summary>
        public static IDisposable SubscribeOnLatest<T>(this IObservable<T> source, Func<T, Task> onNext, Action onCompleted)
        {
            return source.Subscribe(CreateOnLatestWrapper(onNext), exception => { }, onCompleted);
        }

        /// <summary>
        /// Subscribe and retain only the last event while busy processing.
        /// </summary>
        public static IDisposable SubscribeOnLatest<T>(this IObservable<T> source, Action<T> onNext, Action onCompleted)
        {
            return source.Subscribe(CreateOnLatestWrapper<T>(value =>
            {
                onNext(value);
                return Task.CompletedTask;
            }), onCompleted);
        }

        /// <summary>
        /// Subscribe and retain only the last event while busy processing.
        /// </summary>
        public static IDisposable SubscribeOnLatest<T>(this IObservable<T> source, Func<T, Task> onNext, Action<Exception> onError, Action onCompleted)
        {
            return source.Subscribe(CreateOnLatestWrapper(onNext), onError, onCompleted);
        }

        /// <summary>
        /// Subscribe and retain only the last event while busy processing.
        /// </summary>
        public static IDisposable SubscribeOnLatest<T>(this IObservable<T> source, Action<T> onNext, Action<Exception> onError, Action onCompleted)
        {
            return source.Subscribe(CreateOnLatestWrapper<T>(value =>
            {
                onNext(value);
                return Task.CompletedTask;
            }), onError, onCompleted);
        }

        private static Action<T> CreateOnLatestWrapper<T>(Func<T, Task> onNext)
        {
            var context = new SubscribeOnLatestContext<T> { PendingValue = default, Pending = false, Busy = false };

            return async value => {
                lock (context)
                {
                    context.PendingValue = value;
                    if (context.Busy)
                    {
                        context.Pending = true;
                        return;
                    }
                    context.Busy = true;
                }
                while (true)
                {
                    await onNext(value);
                    lock (context)
                    {
                        context.Busy = false;
                        if (!context.Pending)
                            return;
                        value = context.PendingValue;
                        context.Pending = false;
                        context.Busy = true;
                    }
                }
            };
        }

        public class SubscribeOnLatestContext<T>
        {
            public T PendingValue { get; set; }
            public bool Pending { get; set; }
            public bool Busy { get; set; }
        }

        #endregion
    }
}