def FlagsForFile(filename, **kwargs):
    return {
        'flags': [
            '-std=c++14',
            '-framework',
            'GLUT',
            '-framework',
            'OpenGL',
            '-framework',
            'Cocoa'
            ]
    }
