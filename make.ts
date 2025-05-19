#!/usr/bin/env bun
import { readdirSync } from "fs"
import { rm } from "fs/promises"
import { resolve } from "path"
import { Glob, spawn, stdout } from "bun"

const [,, option] = process.argv;

switch(option){
  case "clean":
    await Promise.all(readdirSync(".", { recursive: true }).map(e=>{
      if(typeof e != "string") return;
      if(e.endsWith(".o") || e.endsWith(".d"))
        return rm(e);
    })); break;
  case "link":
    const objects = readdirSync(".", { recursive: true })
      .filter(e=>typeof e == "string" && e.endsWith(".o")) as string[];
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
