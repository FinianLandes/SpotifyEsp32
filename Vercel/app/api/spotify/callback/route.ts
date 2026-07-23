import { NextRequest, NextResponse } from "next/server";
import { createClient } from "redis";

const redis = await createClient({
    url: process.env.REDIS_URL,
    password: process.env.REDIS_PASSWORD,
}).connect();

export async function GET(request: NextRequest) {
    const { searchParams } = new URL(request.url);
    const code = searchParams.get("code");
    const state = searchParams.get("state");

    if (!code || !state) {
        return new NextResponse("Missing code or state", { status: 400 });
    }

    await redis.set(state, code, { EX: 60 });

    const html = `
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <title>Spotify Auth Complete</title>
        <style>
        body {
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            height: 100vh;
            background: #121212;
            color: #1DB954;
            font-family: Arial, sans-serif;
            text-align: center;
        }
        .container {
            max-width: 400px;
            padding: 2em;
            background: #181818;
            border-radius: 12px;
            box-shadow: 0 0 20px rgba(0,0,0,0.4);
        }
        h1 {
            margin-bottom: 0.5em;
        }
        p {
            color: #ccc;
        }
        </style>
    </head>
    <body>
        <div class="container">
        <h1>Authenticated</h1>
        <p>You can close this tab and return to your device.</p>
        </div>
    </body>
    </html>
    `;

    return new NextResponse(html, {
        status: 200,
        headers: { "Content-Type": "text/html" },
    });
}

