const char login_page[] PROGMEM = R"=====(
<HTML>
    <HEAD>
        <TITLE>My first web page</TITLE>
    </HEAD>
    <BODY>
        <CENTER>
            <H1>Spotify Login</H1>
            <a href="https://accounts.spotify.com/authorize?response_type=code&client_id=%s&redirect_uri=%s&scope=ugc-image-upload playlist-read-collaborative playlist-modify-private playlist-modify-public playlist-read-private user-read-playback-position user-read-recently-played user-top-read user-modify-playback-state user-read-currently-playing user-read-playback-state user-read-private user-read-email user-library-modify user-library-read user-follow-modify user-follow-read streaming app-remote-control">Log in to spotify</a>
        </CENTER>
    </BODY>
</HTML>
)=====";
