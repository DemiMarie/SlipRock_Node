{ 'targets':
  [
      {
          'include_dirs': [
              '<!(node -e \'require("nan")\')',
              'SlipRock/src',
          ],
          'target_name': 'bindings',
          'sources': [
              'src/sliprock.cc',
          ],
          'link_dirs': [
              'SlipRock/build/src',
          ],
          'condition': [
              ['os=="linux"', {
                  'cflags': [
                      '-std=c++14',
                      '-Wall',
                      '-Wextra',
                      '-pedantic',
                      '-Werror',
                  ],
              }],
              ['os=="win"', {
                  'sources': [
                      './SlipRock/src/sliprock_windows.c',
                  ],
              }, {
                  'sources': [
                      './SlipRock/src/sliprock.c',
                  ],
                  'libraries': [
                      '-lsodium',
                  ],
              }],
          ]
      }
  ],
}
