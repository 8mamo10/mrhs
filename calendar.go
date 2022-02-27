package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"time"

	aio "github.com/adafruit/io-client-go"
	"google.golang.org/api/calendar/v3"
	"google.golang.org/api/option"
)

const (
	calendarConfigPath    = "./.calendar.json"
	adafruitConfigPath    = "./.adafruit.json"
	credentialPath        = "./.credential.json"
	busy                  = "100"
	notBusy               = "0"
	scheduleFetchInterval = time.Minute * 15
	scheduleCheckInterval = time.Minute * 1
)

type CalendarConfig struct {
	CalenderId string `json:"calendar_id"`
}

type AdafruitConfig struct {
	Username string `json:"username"`
	Key      string `json:"key"`
	Feed     string `json:"feed"`
}

var defaultConfig = CalendarConfig{
	CalenderId: "primary",
}

type Schedule struct {
	StartDateTime time.Time
	EndDateTime   time.Time
	Summary       string
}

type ScheduleList struct {
	Schedules []Schedule
	UpdatedAt time.Time
}

func (s *ScheduleList) dump() {
	log.Println("----------")
	log.Printf("UpdatedAt:%v\n", s.UpdatedAt)
	for _, schedule := range s.Schedules {
		log.Printf("%v,%v,%v\n", schedule.StartDateTime, schedule.EndDateTime, schedule.Summary)
	}
	log.Println("----------")
}

func getCalendarConfig(path string) (CalendarConfig, error) {
	b, err := ioutil.ReadFile(path)
	if err != nil {
		return CalendarConfig{}, fmt.Errorf("failed to read config file. err: %v", err)
	}
	var c CalendarConfig
	err = json.Unmarshal(b, &c)
	if err != nil {
		return CalendarConfig{}, fmt.Errorf("invalid json file. err: %v", err)
	}
	return c, nil
}

func getAdafruitConfig(path string) (AdafruitConfig, error) {
	b, err := ioutil.ReadFile(path)
	if err != nil {
		return AdafruitConfig{}, fmt.Errorf("failed to read config file. err: %v", err)
	}
	var c AdafruitConfig
	err = json.Unmarshal(b, &c)
	if err != nil {
		return AdafruitConfig{}, fmt.Errorf("invalid json file. err: %v", err)
	}
	return c, nil
}

func fetchNextOneDaySchedules(calendarId string) (*ScheduleList, error) {
	ctx := context.Background()
	srv, err := calendar.NewService(ctx, option.WithCredentialsFile(credentialPath))
	// Use this if passing the credentials via environment variables
	//srv, err := calendar.NewService(ctx)
	if err != nil {
		return &ScheduleList{}, fmt.Errorf("unable to retrieve calendar client. err: %v", err)
	}

	from := time.Now().Format(time.RFC3339)
	to := time.Now().AddDate(0, 0, 1).Format(time.RFC3339)
	log.Printf("Duration:%v - %v\n", from, to)

	events, err := srv.Events.List(calendarId).ShowDeleted(false).
		SingleEvents(true).TimeMin(from).TimeMax(to).OrderBy("startTime").Do()
	if err != nil {
		return &ScheduleList{}, fmt.Errorf("unable to retrieve next events: %v", err)
	}
	if len(events.Items) == 0 {
		return &ScheduleList{}, fmt.Errorf("no upcoming events found")
	}

	scheduleList := ScheduleList{}
	for _, item := range events.Items {
		startDateTime, err := time.Parse(time.RFC3339, item.Start.DateTime)
		if err != nil {
			continue
		}
		endDateTime, err := time.Parse(time.RFC3339, item.End.DateTime)
		if err != nil {
			continue
		}
		summary := item.Summary
		s := Schedule{
			StartDateTime: startDateTime,
			EndDateTime:   endDateTime,
			Summary:       summary,
		}
		scheduleList.Schedules = append(scheduleList.Schedules, s)
	}
	scheduleList.UpdatedAt = time.Now()
	return &scheduleList, nil
}

func onMeetingNow(scheduleList *ScheduleList) bool {
	now := time.Now()
	for _, schedule := range scheduleList.Schedules {
		start := schedule.StartDateTime
		end := schedule.EndDateTime
		summary := schedule.Summary
		if summary == "" {
			//log.Printf("%v-%v is not meeting\n", start, end)
			continue
		}
		if now.After(start) && now.Before(end) {
			log.Printf("%v-%v is meeting!!\n", start, end)
			return true
		}
	}
	return false
}

func updateFeed(client *aio.Client, value string) error {
	d := aio.Data{Value: value}
	data, response, err := client.Data.Create(&d)
	if err != nil {
		// TODO: Failed to send data. err: json: cannot unmarshal string into Go struct field Data.id of type int
		// return fmt.Errorf("failed to send data. err: %v", err)
		return nil
	} else {
		log.Printf("Updated value of %v: %v\n", client.Feed.CurrentFeed.Name, data.Value)
		response.Debug()
		return nil
	}
}

func notifyCurrentStatus(client *aio.Client, scheduleList *ScheduleList) error {
	var err error
	if onMeetingNow(scheduleList) {
		log.Println("I am busy now")
		err = updateFeed(client, busy)
	} else {
		log.Println("I am free now")
		err = updateFeed(client, notBusy)
	}
	if err != nil {
		return fmt.Errorf("failed to update feed. err: %v", err)
	}
	return nil
}

func main() {
	calendarConfig, err := getCalendarConfig(calendarConfigPath)
	if err != nil {
		log.Fatalf("Failed to get calendar config. err: %v", err)
		os.Exit(1)
	}
	log.Printf("CalenderId:%s\n", calendarConfig.CalenderId)

	adafruitConfig, err := getAdafruitConfig(adafruitConfigPath)
	if err != nil {
		log.Fatalf("Failed to get adafruit config. err: %v", err)
		os.Exit(1)
	}
	log.Printf("Username:%s\n", adafruitConfig.Username)
	log.Printf("Key:%s\n", adafruitConfig.Key)
	log.Printf("Feed:%s\n", adafruitConfig.Feed)

	client := aio.NewClient(adafruitConfig.Key)
	feed, response, err := client.Feed.Get(adafruitConfig.Feed)
	if err != nil {
		log.Printf("Failed to get feed. err: %v\n", err)
		os.Exit(1)
	}
	response.Debug()
	log.Printf("Last value of %v: %v\n", feed.Name, feed.LastValue)
	log.Printf("%v\n", feed)
	client.SetFeed(feed)

	fetchTicker := time.NewTicker(scheduleFetchInterval)
	defer fetchTicker.Stop()
	checkTicker := time.NewTicker(scheduleCheckInterval)
	defer checkTicker.Stop()

	scheduleList, err := fetchNextOneDaySchedules(calendarConfig.CalenderId)
	if err != nil {
		log.Fatalf("Failed to fetch next one day schedules. err: %v", err)
		os.Exit(1)
	}
	scheduleList.dump()
	err = notifyCurrentStatus(client, scheduleList)
	if err != nil {
		log.Fatalf("Failed to notify current status. err: %v", err)
		os.Exit(1)
	}

	for {
		select {
		case <-fetchTicker.C:
			log.Printf("Fetch the latest schedules every %s\n", scheduleFetchInterval)
			scheduleList, err = fetchNextOneDaySchedules(calendarConfig.CalenderId)
			if err != nil {
				log.Fatalf("Failed to fetch next one day schedules. err: %v", err)
			}
			scheduleList.dump()
		case <-checkTicker.C:
			log.Printf("Check the current status every %s\n", scheduleCheckInterval)
			err := notifyCurrentStatus(client, scheduleList)
			if err != nil {
				log.Fatalf("Failed to notify current status. err: %v", err)
			}
		}
	}
}
