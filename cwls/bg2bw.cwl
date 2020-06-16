#!/usr/bin/env cwl-runner

doc: |
  ![build_status](https://quay.io/repository/wtsicgp/dockstore-cgpbigwig/status)

  A wrapper for the cgpBigWig bg2bw tool.

  bg2bw documentation can be found [here](https://github.com/cancerit/cgpBigWig#bg2bw)

  In order to run the example found in `example/bg2bw.json` please download the relevant reference files
  listed in the [`README`](https://github.com/cancerit/cgpdockstore-cgpbigwig/README.md#Example_reference_files)

  See the [dockstore-cgpbigwig](https://github.com/cancerit/dockstore-cgpbigwig)
  website for more information about this wrapper.

  For queries relating to the underlying software see [cgpBigWig](https://github.com/cancerit/cgpBigWig).

cwlVersion: v1.0

class: CommandLineTool

id: "cgpbigwig-bg2bw"
label: "cgpbigwig-bg2bw"

baseCommand: bg2bw

requirements:
  - class: DockerRequirement
    dockerPull: "quay.io/wtsicgp/dockstore-cgpbigwig:3.0.1"

inputs:
  input_path:
    type: File
    doc: "Path to the input [b|cr]am file"
    inputBinding:
      prefix: --input
      position: 1
  chrom_list:
    type: File
    doc: "Path to chrom.list a .tsv where first two columns are contig name and length (i.e. .fai file)"
    inputBinding:
      prefix: --chromList

outputs:
  output:
    type: stdout

$schemas:
  - https://schema.org/version/latest/schema.rdf

$namespaces:
  s: http://schema.org/

s:codeRepository: https://github.com/cancerit/dockstore-cgpbigwig
s:license: https://spdx.org/licenses/GPL-3.0

s:author:
  - class: s:Person
    s:identifier: https://orcid.org/0000-0002-0407-0386
    s:email: mailto:drj@sanger.ac.uk
    s:name: David Jones

dct:creator:
  "@id": "https://orcid.org/0000-0002-0407-0386"
  foaf:name: David Jones
  foaf:mbox: "drj@sanger.ac.uk"
