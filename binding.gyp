{
    "targets": [
        {
            "target_name": "parser",
            "sources": [ "src/parser.cc" ],
            "cflags_cc": [ "-fexceptions" ],
            "cflags!": [ '-fno-exceptions' ],
            "cflags_cc!": [ '-fno-exceptions' ],
            "conditions": [
                ['OS=="mac"', {
                    "xcode_settings": {
                    "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
                    }
                }]
            ],
            "include_dirs" : [
                "<!(node -e \"require('nan')\")"
            ]
        }
    ]
}
