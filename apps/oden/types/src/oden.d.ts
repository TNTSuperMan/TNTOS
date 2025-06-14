declare module "oden" {
    export function getTime(): {
        day: number;
        daylight: number;
        hour: number;
        minute: number;
        month: number;
        nanosecond: number;
        pad1: number;
        pad2: number;
        second: number;
        timezone: number;
        year: number;
    }

    // File API
    export function readFile (path: string): string;
    export function writeFile(path: string, input: string | ArrayBuffer | Buffer | DataView | Int8Array | Uint8Array | Uint8ClampedArray | Int16Array | Uint16Array | Int32Array | Uint32Array | Float32Array | Float64Array): void;
    
    // Window API
    export function openWindow(width: number, height: number, x: number, y: number, title: string): number;
    export function closeWindow  (id: number): void;
    export function redrawWindow (id: number);
    export function drawString   (id: number, x: number, y: number, color: number, str: string): void;
    export function drawRectangle(id: number, x: number, y: number, width: number, height: number, color: number): void;
    export function drawLine     (id: number, x0: number, y0: number, x1: number, y1: number, color: number): void;

    // Timer API
    export function createTimer(type: number, timer_value: number, timeout_ms: number): number;

    // Event API
    export function readEvent(): AppEvent;
    export type AppEvent = {
        type: "quit";
    } | {
        type: "mouse_move";
        x: number; y: number;
        dx: number; dy: number;
        buttons: number;
    } | {
        type: "mouse_button";
        x: number; y: number;
        press: boolean;
        button: number;
    } | {
        type: "timer";
        timeout: number;
        value: number;
    } | {
        type: "keypush";
        modifier: number;
        keycode: number;
        ascii: string;
        press: boolean;
    }
}
