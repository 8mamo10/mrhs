package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"time"

	"google.golang.org/api/calendar/v3"
)

const (
	configPath = "./.calendar.json"
)

type config struct {
	CalendarID string `json:"calendar_id"`
}

var defaultConfig = config{
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

func getConfig(path string) (config, error) {
	b, err := ioutil.ReadFile(path)
	if err != nil {
		return config{}, fmt.Errorf("failed to read config file. err: %v", err)
	}
	var c config
	err = json.Unmarshal(b, &c)
	if err != nil {
		return config{}, fmt.Errorf("invalid json file. err: %v", err)
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
	config, err := getConfig(configPath)
	if err != nil {
		log.Fatalf("Failed to get config. err: %v", err)
	}
	fmt.Printf("CalenderId:%s\n", config.CalendarID)

	scheduleList, err := fetchNextOneDaySchedules(config.CalendarID)
	if err != nil {
		log.Fatalf("Failed to fetch next one day schedules. err: %v", err)
	}
	scheduleList.dump()

	if onMeetingNow(scheduleList) {
		fmt.Println("I am busy now")
	} else {
		fmt.Println("I am free now")
	}
}