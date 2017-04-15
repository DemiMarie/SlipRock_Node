{ 'targets':
  [
      {
          'include_dirs': [
              '<!(node -e \'require("nan")\')',
              'SlipRock/src',
          ],
          'target_name': 'sliprock',
          'sources': [
              'src/sliprock.cc',
          ],
          'conditions': [
              ['OS=="win"', {
                  'sources': [
                      'SlipRock/src/sliprock_windows.c',
                  ],
              }, {
                  'sources': [
                      'SlipRock/src/sliprock.c',
                  ],
                  'libraries': [
                      '-lsodium',
                  ],
                  'cflags': [
                      '-Wall',
                      '-Wextra',
                      '-pedantic',
                      '-Werror',
                  ],
             }],
          ]
      }
  ],
}
