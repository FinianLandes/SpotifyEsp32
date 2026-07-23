import { createClient } from 'redis';
import { NextRequest, NextResponse } from 'next/server';

export const GET = async (req: NextRequest) => {
    const state = req.nextUrl.searchParams.get("state");
    if (!state) return NextResponse.json({ code: null });

    const redis = createClient({
        url: process.env.REDIS_URL,
        password: process.env.REDIS_PASSWORD,
    });

    try {
        await redis.connect();
        const code = await redis.get(state);
        if (code) await redis.del(state);
        await redis.quit();
        const body = JSON.stringify({ code: code ?? null });

        const res = new NextResponse(body, { status: 200 });
        res.headers.set("Content-Type", "application/json");
        res.headers.set("Connection", "close");
        res.headers.set("Content-Length", body.length.toString());

        return res;
    } catch (err) {
        if (redis.isOpen) await redis.quit();
        const body = JSON.stringify({ code: null, error: (err as Error).message });
        const res = new NextResponse(body, { status: 500 });
        res.headers.set("Content-Type", "application/json");
        res.headers.set("Connection", "close");
        res.headers.set("Content-Length", body.length.toString());
        return res;
    }
};