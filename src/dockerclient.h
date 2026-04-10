/*
    SPDX-FileCopyrightText: 2026 Salman Ahmed <salman.sc829@gmail.com>

    SPDX-License-Identifier: Apache-2.0
*/

#ifndef DOCKER_CLIENT_H
#define DOCKER_CLIENT_H

#include <QObject>
#include <QString>
#include <QList>

class QProcess;

struct ContainerInfo {
    QString id;
    QString name;
    QString image;
    QString status;
    QString ports;
};

enum class DockerAvailability {
    Unknown,
    Available,
    NotFound,
    DaemonDown,
    PermissionDenied,
    Error
};

class DockerClient : public QObject
{
    Q_OBJECT

public:
    explicit DockerClient(QObject *parent = nullptr);
    ~DockerClient() override;

    void checkAvailability();
    void listContainers(bool showAll = false);
    void startContainer(const QString &containerId);
    void stopContainer(const QString &containerId);
    void removeContainer(const QString &containerId);
    void inspectContainer(const QString &containerId);

Q_SIGNALS:
    void availabilityChanged(DockerAvailability availability, const QString &errorMessage);
    void containersUpdated(const QList<ContainerInfo> &containers);
    void commandFinished(const QString &action, const QString &containerId, bool success, const QString &output);
    void inspectResult(const QString &containerId, const QString &jsonText);

private:
    void runContainerCommand(const QString &action, const QString &containerId, const QStringList &args);
    DockerAvailability classifyError(const QString &stdErr) const;
    QList<ContainerInfo> parseContainerOutput(const QByteArray &data) const;
};

#endif // DOCKER_CLIENT_H
