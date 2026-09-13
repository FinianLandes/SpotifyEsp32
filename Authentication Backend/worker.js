export default {
async fetch(request, env) {
  const url = new URL(request.url);

  if (url.pathname === "/callback" || url.pathname === "/api/spotify/callback") {
    const code = url.searchParams.get("code");
    const state = url.searchParams.get("state");

    if (!code || !state) {
      console.error("Callback missing code or state");
      return new Response("Missing code or state", { status: 400 });
    }
    console.log(`[Callback] Received state: ${state}, storing code in Redis...`);

    const redisUrl = `${env.UPSTASH_REDIS_REST_URL}/set/${encodeURIComponent(state)}/${encodeURIComponent(code)}?EX=60`;
    const redisResponse = await fetch(redisUrl, {
        headers: {
          Authorization: `Bearer ${env.UPSTASH_REDIS_REST_TOKEN}`,
        },
      });

      if (!redisResponse.ok) {
        console.error("[Callback] Failed to store code in Redis:", await redisResponse.text());
        return new Response("Error storing state in Redis", { status: 500 });
      }

      console.log("[Callback] Successfully stored code in Redis.");

      const html = `<!DOCTYPE html>
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
  h1 { margin-bottom: 0.5em; }
  p { color: #ccc; }
  </style>
</head>
<body>
  <div class="container">
      <h1>Authenticated</h1>
      <p>You can close this tab and return to your device.</p>
  </div>
</body>
</html>`;

      return new Response(html, {
        status: 200,
        headers: { "Content-Type": "text/html" },
      });
    }

    if (url.pathname === "/api/spotify/get_code") {
      const state = url.searchParams.get("state");

      if (!state) {
        return Response.json({ code: null });
      }

      try {
        const redisPipelineUrl = `${env.UPSTASH_REDIS_REST_URL}/pipeline`;
        const redisPipelineResponse = await fetch(redisPipelineUrl, {
          method: "POST",
          headers: {
            Authorization: `Bearer ${env.UPSTASH_REDIS_REST_TOKEN}`,
            "Content-Type": "application/json",
          },
          body: JSON.stringify([
            ["GET", state],
            ["DEL", state],
          ]),
        });

        const data = await redisPipelineResponse.json();
        const code = data[0]?.result || null;

        if (code) {
          console.log(`[Polling] Code retrieved for state: ${state}`);
        }

        return Response.json(
          { code },
          {
            status: 200,
            headers: {
              "Content-Type": "application/json",
              Connection: "close",
            },
          }
        );
      } catch (err) {
        console.error("[Polling] Error fetching code from Redis:", err.message);
        return Response.json(
          { code: null, error: err.message },
          { status: 500 }
        );
      }
    }

    return new Response("Not Found", { status: 404 });
  },
};