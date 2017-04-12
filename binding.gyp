{ 'targets': [
    {
        'include_dirs': [
            '<!(node -e \'require("nan")\')',
            '../SlipRock/src',
            ],
        'target_name': 'bindings',
        'sources': [
            'src/sliprock.cc',
        ],
        'condition': [
            'OS=="linux"': {
                'cflags': [
                    '-fcolor-diagnostics',
                    '-std=c++14'
                    ]
                }
            ]
        }
    ],
}
