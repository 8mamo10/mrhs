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

type config struct {
	CalendarID string `json:"calendar_id"`
}

var defaultConfig = config{
	CalendarID: "primary",
}

type schedule struct {
	StartDateTime time.Time
	EndDateTime   time.Time
	Summary       string
}

type scheduleList struct {
	Schedules []schedule
	UpdatedAt time.Time
}

func (s *scheduleList) dump() {
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

func fetchNextOneDaySchedules(calendarId string) (scheduleList, error) {
	ctx := context.Background()
	srv, err := calendar.NewService(ctx)
	if err != nil {
		return scheduleList{}, fmt.Errorf("unable to retrieve calendar client. err: %v", err)
	}

	t := time.Now().Format(time.RFC3339)
	fmt.Printf("Now:%v\n", t)
	from := time.Now().Format(time.RFC3339)
	to := time.Now().AddDate(0, 0, 1).Format(time.RFC3339)
	fmt.Printf("Duration:%v - %v\n", from, to)

	events, err := srv.Events.List(calendarId).ShowDeleted(false).
		SingleEvents(true).TimeMin(from).TimeMax(to).OrderBy("startTime").Do()
	if err != nil {
		return scheduleList{}, fmt.Errorf("unable to retrieve next events: %v", err)
	}
	if len(events.Items) == 0 {
		return scheduleList{}, fmt.Errorf("no upcoming events found")
	}

	scheduleList := scheduleList{}
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
		s := schedule{
			StartDateTime: startDateTime,
			EndDateTime:   endDateTime,
			Summary:       summary,
		}
		scheduleList.Schedules = append(scheduleList.Schedules, s)
	}
	return scheduleList, nil
}

func main() {
	const configPath = "./.calendar.json"
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
}
