#!/usr/bin/env cwl-runner

doc: |
  ![build_status](https://quay.io/repository/wtsicgp/dockstore-cgpbigwig/status)

  A wrapper for the cgpBigWig bam2bwbases tool.

  bam2bwbases documentation can be found [here](https://github.com/cancerit/cgpBigWig#bam2bwbases)

  In order to run the example found in `example/bam2bwbases.json` please download the relevant reference files
  listed in the [`README`](https://github.com/cancerit/cgpdockstore-cgpbigwig/README.md#Example_reference_files)
  and change the reference path in bam2bwbases.json

  See the [dockstore-cgpbigwig](https://github.com/cancerit/dockstore-cgpbigwig)
  website for more information about this wrapper.

  For queries relating to the underlying software see [cgpBigWig](https://github.com/cancerit/cgpBigWig).

cwlVersion: v1.0

class: CommandLineTool

id: "cgpbigwig-bam2bwbases"
label: "cgpbigwig-bam2bwbases"

baseCommand: bam2bwbases

requirements:
  - class: DockerRequirement
    dockerPull: "quay.io/wtsicgp/dockstore-cgpbigwig:3.0.1"
inputs:
  input_path:
    type: File
    inputBinding:
      prefix: --input
      position: 1
    secondaryFiles: .bai
  filter:
    type: int?
    inputBinding:
      prefix: --filter
  region:
    type: string?
    inputBinding:
      prefix: --region
  reference:
    type: File?
    inputBinding:
      prefix: --reference
  overlap:
    type: boolean
    doc: "Flag to check for overlapping reads"
    inputBinding:
      prefix: --overlap


outputs:
  output:
    type: stdout
    
stdout: bam2bwbases.out.bw

$namespaces:
    s: http://schema.org/

$schemas:
    - https://schema.org/version/latest/schema.rdf

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
