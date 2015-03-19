import sys
import argparse
import struct

class Vertex:
    def __init__(self, line):
        values = line.split(' ')[1:]
        values = [float(v) for v in values]
        if len(values) == 3:
            self.values = values + [1.0]
        elif len(values) == 4:
            self.values = values
        else:
            raise Exception('Invalid number of values for vertex')

class Normal:
    def __init__(self, line):
        values = line.split(' ')[1:]
        values = [float(v) for v in values]
        if len(values) != 3:
            raise Exception('Invalid number of values for normal')
        self.values = values

class Face:
    class Point:
        def __init__(self):
            self.vertexIndex = 0
            self.normalIndex = 0
            self.textureIndex = 0

    def __init__(self, line):
        self.format = ''
        elements = line.split(' ')[1:]
        self.points = []

        if len(elements) != 3: raise Exception('Invalid number of points on face')

        for i in range(len(elements)):
            keys = elements[i].split('/')
            point = Face.Point()

            if keys[0] != '':
                if i == 0: self.format = self.format + 'v'
                point.vertexIndex = int(keys[0]) - 1
            else:
                raise Exception('Face did not contain vertex index')

            if keys[1] != '':
                if i == 0: self.format = self.format + 't'
                point.textureIndex = int(keys[1]) - 1

            if keys[2] != '':
                if i == 0: self.format = self.format + 'n'
                point.normalIndex = int(keys[2]) - 1
            else:
                raise Exception('Face did not contain normal index')

            self.points.append(point)


def convert_to_objects(data):
    lines = data.split('\n')

    vertexes = []
    normals = []
    faces = []
    texture = []

    for line in lines:

        if line.startswith('#'):
            continue
        elif line.startswith('v '):
            vertexes.append(Vertex(line))
        elif line.startswith('vn '):
            normals.append(Normal(line))
        elif line.startswith('f '):
            faces.append(Face(line))
            if len(faces) > 1:
                if faces[-1].format != faces[-2].format:
                    raise Exception('Format of face changed during file')

    if len(faces) == 0:
        raise Exception('No faces found')
    if len(vertexes) == 0:
        raise Exception('No vertexes found')
    if len(normals) == 0:
        raise Exception('No normals found')

    return (faces[0].format, faces, vertexes, normals, texture)

def formatDataVn(faces, vertexes, normals):
    indices = []
    data = []

    for f in faces:
        for p in f.points:
            value = vertexes[p.vertexIndex].values + normals[p.normalIndex].values
            if not value in data:
                index = len(data)
                data.append(value)
                indices.append(index)
                trace('Adding new indice: data index ' + str(index) + ', value ' + str(value))
            else:
                index = data.index(value)
                indices.append(index)
                trace('Reusing value from data index: ' + str(index) + ', value ' + str(value))

    return indices, data

def serializeVn32b(indices, attributeData, fo):

    data = ''
    # Attribute format
    data += struct.pack('<cccc', 'v', 'n', ' ', ' ')

    # Size of indice element
    indicesElementFormat = ''
    if len(data) < struct.unpack('H', '\xff\xff'):
        data += struct.pack('<B', 2)
        indicesElementFormat = '<H'
        trace('Size of indice: 2B')
    else:
        data += struct.pack('<B', 4)
        indicesElementFormat = '<I'
        trace('Size of indice: 4B')

    # Number of indices
    data += struct.pack('<I', len(indices))

    # Indices
    for i in indices:
        data += struct.pack(indicesElementFormat, i)

    # Attribute data
    data += struct.pack('<I', len(attributeData))
    for d in attributeData:
        data += struct.pack('<fffffff', d[0], d[1], d[2], d[3], d[4], d[5], d[6])
    fo.write(data)

    trace('Size of data: ' + str(len(data) + 1) # +1 for version which was set in main
          + ', number of indices: ' + str(len(indices))
          + ', number of attribute elements: ' + str(len(attributeData))
          )

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Converts .obj into SToolbox .sm format')
    parser.add_argument('-i', '--input_file', type=str, help='Input file', required=True)
    parser.add_argument('-o', '--output_file', type=str, help='Output file', required=True)
    parser.add_argument('-v', action='store_true', help='Verbose output')
    args = parser.parse_args()
    
    if args.v:
        def verbose(str):
            print str
    else:
        verbose = lambda * a: None
    global trace
    trace = verbose

    fi = open(args.input_file, 'r')
    fo = open(args.output_file, 'wb')
    format, faces, vertexes, normals, texture = convert_to_objects(fi.read())

    protocolVersion = 1
    fo.write(struct.pack('<b', protocolVersion))

    if format == 'vn':
        indices, data = formatDataVn(faces, vertexes, normals)
        serializeVn32b(indices, data, fo)
    else:
        raise Exception("Invalid format")
