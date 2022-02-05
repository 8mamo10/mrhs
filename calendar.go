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

type Schedule struct {
	StartDateTime time.Time
	EndDateTime   time.Time
	Summary       string
}

type ScheduleList struct {
	Schedules []Schedule
	UpdatedAt time.Time
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

func main() {
	const configPath = "./.calendar.json"
	c, err := getConfig(configPath)
	if err != nil {
		log.Fatalf("Failed to get config. err: %v", err)
	}
	fmt.Printf("CalenderId:%s\n", c.CalendarID)

	ctx := context.Background()
	srv, err := calendar.NewService(ctx)
	if err != nil {
		log.Fatalf("Unable to retrieve Calendar client: %v", err)
	}

	t := time.Now().Format(time.RFC3339)
	fmt.Printf("Now:%v\n", t)
	/*
		events, err := srv.Events.List(c.CalendarID).ShowDeleted(false).
			SingleEvents(true).TimeMin(t).MaxResults(10).OrderBy("startTime").Do()
	*/
	from := time.Now().Format(time.RFC3339)
	to := time.Now().AddDate(0, 0, 1).Format(time.RFC3339)
	fmt.Printf("Duration:%v - %v\n", from, to)
	events, err := srv.Events.List(c.CalendarID).ShowDeleted(false).
		SingleEvents(true).TimeMin(from).TimeMax(to).OrderBy("startTime").Do()

	if err != nil {
		log.Fatalf("Unable to retrieve next ten of the user's events: %v", err)
	}
	if len(events.Items) == 0 {
		log.Fatal("No upcoming events found")
	}
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
		fmt.Printf("%v,%v,%v\n", s.StartDateTime, s.EndDateTime, s.Summary)
	}
}
