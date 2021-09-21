using System;
using System.Collections.Generic;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Calendar.v3;
using Google.Apis.Calendar.v3.Data;
using Google.Apis.Services;
using Google.Apis.Util.Store;
using System.IO;
using System.Threading;

namespace WhiteMagic.PanelClock
{
    public class Calendar
    {
        public Calendar()
        {

        }

        static string[] Scopes = { CalendarService.Scope.CalendarReadonly };
        static string ApplicationName = "PanelClock";

        public class Event
        {
            public string Calendar { get; set; }
            public DateTime When { get; set; }
            public bool AllDay { get; set; }
            public string Message { get; set; }

            public override string ToString()
            {
                return $"{When} {Message}  ({Calendar})";
            }
        }

        public List<Event> GetEventsForDate(DateTime when)
        {
            UserCredential credential;

            using (var stream =
                new FileStream("credentials.json", FileMode.Open, FileAccess.Read))
            {
                // The file token.json stores the user's access and refresh tokens, and is created
                // automatically when the authorization flow completes for the first time.
                string credPath = "token.json";
                credential = GoogleWebAuthorizationBroker.AuthorizeAsync(
                    GoogleClientSecrets.Load(stream).Secrets,
                    Scopes,
                    "user",
                    CancellationToken.None,
                    new FileDataStore(credPath, true)).Result;
                Console.WriteLine("Credential file saved to: " + credPath);
            }

            // Create Google Calendar API service.
            var service = new CalendarService(new BaseClientService.Initializer()
            {
                HttpClientInitializer = credential,
                ApplicationName = ApplicationName,
            });

            var calendars = service.CalendarList.List().Execute();
            var items = new List<Event>();
            foreach (var calendar in calendars.Items)
            {
                // Define parameters of request.
                EventsResource.ListRequest request = service.Events.List(calendar.Id);
                request.TimeMin = DateTime.Now.AddHours(-2);
                request.TimeMax = DateTime.Now.AddDays(1);
                request.ShowDeleted = false;
                request.SingleEvents = true;
                request.MaxResults = 30;
                request.OrderBy = EventsResource.ListRequest.OrderByEnum.StartTime;

                // List events.
                Events events = request.Execute();

                foreach (var evt in events.Items)
                {
                    items.Add(
                        new Event
                        {
                            Calendar = calendar.Summary,
                            When = evt.Start.DateTime ?? DateTime.Parse(evt.Start.Date),
                            AllDay = !evt.Start.DateTime.HasValue,
                            Message = evt.Summary
                        });

                }
            }
            return items;
        }
    }
}