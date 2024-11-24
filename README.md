# Solar Home Automation ESP32

A cheap home automation project, (for solar cells and power regulation)  
Based upon an ESP32 Wroom dev board  
The idea here is to Monitor power production in winter time.

If enough power turn on a cheap heater.  
If power drops, turn it off for a minimal 5-minute rest (a cool-down period, for the Elchaepo device).

And don't keep it on longer then a certain period, if so cool down and repeat.  
Also turns of if not enough power.

Later i got some more ideas about, it i wanted to see it on a website my current solar production.  
Sink with an NTP time server (done !)  
Use a file cache for faster serving web server (done)  
Restart the webserver to better handle single session using a watchdog (done).  
Get a stable webserver on ESP32 (this was quite a lot of work) done!!

**Still planned**  (soon, next weeks)  
Adding extra electronics (bought and arrived) with some more sensors (web page allready contains them)  
It can do ping detection for a phone, and thereby detecting if your home.  
It has knowledge of weekdays time etc,.. so create some advanced automatic light turn on (when away or arriving).

Future:

Maybe do something with my **solar edge...**  
While monitoring my SolarEdge system's performance, I've noticed some peculiar patterns during periods of reported "network congestion."  
Despite having capacity for additional power consumption, my P1 meter consistently plateaus at specific output levels...  
An interesting correlation that seems to warrant closer examination.  
Grid operators, understandably focused on network stability, implement these limitations through sophisticated management systems,  
though the economic implications for solar investors remain intriguing.  
One might ponder the curious paradox: these entities market themselves with environmental green, yet seem particularly attentive to a different shade of green ($$$) altogether.  
The situation becomes especially thought-provoking when considering that excess solar power, which could potentially benefit neighboring consumers and promote true sustainability.  
It remains unutilized during these "congestion" periods. The data patterns suggest a complex interplay between reported congestion and actual system capacity.  
Raising questions about the current grid management framework.  
My C++ monitoring system continues to collect data, revealing patterns that might interest those analyzing grid integration metrics  
particularly those curious about the flow of both power and ($$$) profits.

Or mMaybe do something with my **washing machines..** but not sure i have no documentation of api's ... (well not yet).

# hardware list

> averaged prices, homewizard is a bit pricy perhaps, but the ease of their wifi i licked it.
> 
> They also had a nice android app to see live data so not a bad deal.

| Item | Price (€) |
| --- | --- |
| Sensor board BH1750 | 2.25 |
| Sensor board BME280 | 6.50 |
| OLED display 128\*64 | 15.00 |
| ESP32 Wroom | 8.00 |
| Home Wizard P1 | 27.00 |
| HomeWizard switch | 27.95 |
| HomeWizard switch | 27.95 |
| HomeWizard switch | 27.95 |
| **Total** | **142.60** |

---

# Wiring