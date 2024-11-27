#pragma once

#include "main.h"
#include <shoyu-shell-client-protocol.h>

#define SHOYU_SHELL_GTK_MONITOR_KEY "shoyu-shell-gtk4-monitor"

struct shoyu_shell_output *shoyu_shell_gtk_get_output(GdkMonitor *monitor);
