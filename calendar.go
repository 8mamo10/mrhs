package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"time"

	aio "github.com/adafruit/io-client-go"
	"google.golang.org/api/calendar/v3"
)

const (
	calendarConfigPath = "./.calendar.json"
	adafruitConfigPath = "./.adafruit.json"
)

type CalendarConfig struct {
	CalendarID string `json:"calendar_id"`
}

type AdafruitConfig struct {
	Username string `json:username`
	Key      string `json:key`
}

var defaultConfig = CalendarConfig{
	CalendarID: "primary",
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
	fmt.Printf("UpdatedAt:%v\n", s.UpdatedAt)
	for _, schedule := range s.Schedules {
		fmt.Printf("%v,%v,%v\n", schedule.StartDateTime, schedule.EndDateTime, schedule.Summary)
	}
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

func fetchNextOneDaySchedules(calendarId string) (ScheduleList, error) {
	ctx := context.Background()
	srv, err := calendar.NewService(ctx)
	if err != nil {
		return ScheduleList{}, fmt.Errorf("unable to retrieve calendar client. err: %v", err)
	}

	from := time.Now().Format(time.RFC3339)
	to := time.Now().AddDate(0, 0, 1).Format(time.RFC3339)
	fmt.Printf("Duration:%v - %v\n", from, to)

	events, err := srv.Events.List(calendarId).ShowDeleted(false).
		SingleEvents(true).TimeMin(from).TimeMax(to).OrderBy("startTime").Do()
	if err != nil {
		return ScheduleList{}, fmt.Errorf("unable to retrieve next events: %v", err)
	}
	if len(events.Items) == 0 {
		return ScheduleList{}, fmt.Errorf("no upcoming events found")
	}

	scheduleList := ScheduleList{}
	fmt.Println("Upcoming events:")
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
	return scheduleList, nil
}

func onMeetingNow(scheduleList ScheduleList) bool {
	now := time.Now()
	for _, schedule := range scheduleList.Schedules {
		start := schedule.StartDateTime
		end := schedule.EndDateTime
		summary := schedule.Summary
		if summary == "" {
			fmt.Printf("%v-%v is not meeting\n", start, end)
			continue
		}
		if now.After(start) && now.Before(end) {
			fmt.Printf("%v-%v is meeting!!\n", start, end)
			return true
		}
	}
	return false
}

func main() {
	calendarConfig, err := getCalendarConfig(calendarConfigPath)
	if err != nil {
		log.Fatalf("Failed to get calendar config. err: %v", err)
	}
	fmt.Printf("CalenderId:%s\n", calendarConfig.CalendarID)

	adafruitConfig, err := getAdafruitConfig(adafruitConfigPath)
	if err != nil {
		log.Fatalf("Failed to get adafruit config. err: %v", err)
	}
	fmt.Printf("Username:%s\n", adafruitConfig.Username)
	fmt.Printf("Key:%s\n", adafruitConfig.Key)

	scheduleList, err := fetchNextOneDaySchedules(calendarConfig.CalendarID)
	if err != nil {
		log.Fatalf("Failed to fetch next one day schedules. err: %v", err)
	}
	scheduleList.dump()

	if onMeetingNow(scheduleList) {
		fmt.Println("I am busy now")
	} else {
		fmt.Println("I am free now")
	}

	client := aio.NewClient(adafruitConfig.Key)
	fmt.Printf("client: %v\n", client)
	feeds, response, err := client.Feed.All()
	if err != nil {
		fmt.Println("Failed to get all feed")
	}
	fmt.Printf("feeds: %v\n", feeds[0].Name)
	response.Debug()

	feed := feeds[0]
	fmt.Printf("feed: %v\n", feed.LastValue)
}
