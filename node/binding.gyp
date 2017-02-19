{
    "targets": [
        {
            "target_name": "ltc2983",
            "sources": ["../src/ltc2983.cc"],
            "include_dirs": ["../include"],
            "libraries": ["-I/usr/local/include -L/usr/local/lib -lwiringPi"],
            "cflags_cc":['-fexceptions']
        }
    ]
}
