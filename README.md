# Spotify-Console
A Spotify player for Linux terminals.

## Building
In `src/`, run `make`. The binary file `spotifyConsole.exe` will be output.

## Running
The client secret for a Spotify Developer app must be put in a file called `src/CLIENT_SECRET`. Currently, only the client secret for my app will work, as the client ID is hardcoded. This will be changed in the future.

## Issues
Spotify authentication lasts a limited time (I believe 30 minutes). After this, authentication needs to be refreshed. This application does not currently do this automatically, so it will have to be restarted when this happens instead.

