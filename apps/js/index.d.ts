declare function print(...args: unknown[]): undefined;
declare function stats(): {
    size: number;
    lwm: number;
    css: number;
    brk: number;
}
declare function readFile(path: string): string;
declare function writeFile(path: string, input: string): undefined;
