#!/usr/bin/env bun
import { rm } from "fs/promises"
import { $, Glob, spawn, stdout } from "bun"

const [,, option] = process.argv;

switch(option){
  case "clean":
    await $`rm -r **/.*.d`
    for await (const e of new Glob("**/*.o").scan()){
      rm(e);
    } break;
  case "link":
    const objects = Array.from(new Glob("**/*.o").scanSync());
    await spawn({cmd:["ld.lld", ...(process.env.LDFLAGS??"").split(" "),
        "--entry", "KernelMain",
        "-z", "norelro",
        "--image-base", "0x100000",
        "--static",
        "-o", "kernel.elf",
        ...objects,
        "-lc", "-lc++", "-lc++abi", "-lm", "-lfreetype"]}).exited;
    break;
  case "objects":
    stdout.write(Array.from(new Glob("**/*.{c,cpp}").scanSync()).map(e=>e.replace(/\.[\w]*$/,".o")).join(" "));
    break;
}
