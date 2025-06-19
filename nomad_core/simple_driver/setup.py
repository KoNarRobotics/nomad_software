from setuptools import find_packages, setup

package_name = 'simple_driver'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='lemonx',
    maintainer_email='dudzinski.patryk.02@gmail.com',
    description='TODO: Package description',
    license='MIT',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [ 
          'sd_node = simple_driver.simple_driver:main',
          'rc_switch = simple_driver.rc_switch:main', 
          'rc_driver = simple_driver.rc_driver:main',
        ],
    },
)
