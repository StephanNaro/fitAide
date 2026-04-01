# fitAide

A minimalist, no-nonsense workout logger built with Qt6 and SQLite for personal use.

Designed for simple home/garage gym sessions with dumbbells or fixed equipment. The app guides you through your exercises one by one, with a clean rest timer and a preview of the next exercise during the final rest period.

## Features

- **First-run setup**: Automatically prompts for database location, initial exercises, and workout settings.
- **Sequential workout flow**:
  - One exercise at a time with large heading, description, and optional image.
  - Rep selection via clickable buttons (min to max reps, configurable).
  - Automatic rest timer between sets.
  - During the final rest of an exercise, a preview of the next exercise appears.
- **Weight tracking**:
  - Current weight (auto-loaded from previous Next weight).
  - Next weight field — change it only when you decide to progress.
  - Weights carry over sensibly between sessions.
- **Settings**:
  - Number of sets (2–5)
  - Min/Max reps per set
  - Rest time between sets (120–300 seconds)
- **Data persistence**: All workout history saved in a local SQLite database (`fitAide.db` or `fitAideDev.db` in development).

## Screenshots

A workout in progress, with the recently completed sets at the top, and information about the upcoming exercise below.
![Workout in Progress](resources/screenshot.jpg "Workout in Progress").

## Usage

1. Run the app.
2. On first launch, select/create your database file and add your exercises + settings.
3. Each session:
   - Click rep buttons to log completed sets.
   - Rest timer starts automatically after each set.
   - On the last set, the next exercise preview appears during rest.
   - After the final rest finishes, the workout is saved and the app closes.

**Tip**: Change the "Next weight" field at the end of a workout only when you're ready to increase the load for the next session.

## Future / Possible Improvements

- Warmup weight tracking
- BenchNotch / rack position support
- Notes field per exercise per workout
- Muscle group categorization
- Toggle exercises active/inactive
- Ability to add 1–2 extra sets mid-workout
- Workout review / history viewer (instead of immediate close after last exercise)
- Automatic weight progression suggestions
- Add audible countdown finale

These are nice-to-haves. The core logging flow is intentionally kept simple and focused.

## Technical Notes

- Built with **Qt 6** and **SQLite3**.
- Single-user design — no multi-user or cloud sync.
- Database schema supports future fields without breaking changes.
- Development build uses `fitAideDev.db` and separate registry/settings key.

## Building

```bash
qmake -r
make
# or open in Qt Creator
```

## License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0). See [LICENSE](LICENSE) for details.
