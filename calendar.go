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
	fmt.Println("CalenderId: ", c.CalendarID)

	ctx := context.Background()
	srv, err := calendar.NewService(ctx)
	if err != nil {
		log.Fatalf("Unable to retrieve Calendar client: %v", err)
	}
	t := time.Now().Format(time.RFC3339)
	events, err := srv.Events.List(c.CalendarID).ShowDeleted(false).
		SingleEvents(true).TimeMin(t).MaxResults(10).OrderBy("startTime").Do()
	if err != nil {
		log.Fatalf("Unable to retrieve next ten of the user's events: %v", err)
	}
	fmt.Println("Upcoming events:")
	if len(events.Items) == 0 {
		fmt.Println("No upcoming events found.")
	} else {
		for _, item := range events.Items {
			date := item.Start.DateTime
			if date == "" {
				date = item.Start.Date
			}
			fmt.Printf("%v (%v)\n", item.Summary, date)
		}
	}
}
